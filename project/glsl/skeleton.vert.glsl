#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments


uniform mat4 u_BindMats[100];
uniform mat4 u_transformationMats[100];

uniform vec4 u_dualQuatT[100];  // Initial dual quaternion
uniform vec4 u_dualQuatR[100];
uniform vec4 u_transformationDualQuatT[100];
uniform vec4 u_transformationDualQuatR[100];


in ivec2 vs_vertInfluenceIDs;
in vec2 vs_vertInfluenceWeights;

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

in vec4 vs_Pos;
in vec4 vs_Col;

out vec4 fs_Col;

struct DualQuat
{
    // dq = qR + e*qT
    vec4 qT;
    vec4 qR;
};

mat4 inverse_mat4(mat4 m)
{
    float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

    float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

    float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

    float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
    float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

    float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
    float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

    float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
    float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

    const vec4 SignA = vec4( 1.0, -1.0,  1.0, -1.0);
    const vec4 SignB = vec4(-1.0,  1.0, -1.0,  1.0);

    vec4 Fac0 = vec4(Coef00, Coef00, Coef02, Coef03);
    vec4 Fac1 = vec4(Coef04, Coef04, Coef06, Coef07);
    vec4 Fac2 = vec4(Coef08, Coef08, Coef10, Coef11);
    vec4 Fac3 = vec4(Coef12, Coef12, Coef14, Coef15);
    vec4 Fac4 = vec4(Coef16, Coef16, Coef18, Coef19);
    vec4 Fac5 = vec4(Coef20, Coef20, Coef22, Coef23);

    vec4 Vec0 = vec4(m[1][0], m[0][0], m[0][0], m[0][0]);
    vec4 Vec1 = vec4(m[1][1], m[0][1], m[0][1], m[0][1]);
    vec4 Vec2 = vec4(m[1][2], m[0][2], m[0][2], m[0][2]);
    vec4 Vec3 = vec4(m[1][3], m[0][3], m[0][3], m[0][3]);

    vec4 Inv0 = SignA * (Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    vec4 Inv1 = SignB * (Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    vec4 Inv2 = SignA * (Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    vec4 Inv3 = SignB * (Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

    mat4 Inverse = mat4(Inv0, Inv1, Inv2, Inv3);

    vec4 Row0 = vec4(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

    float Determinant = dot(m[0], Row0);

    Inverse /= Determinant;

    return Inverse;
}

DualQuat DualQuatNormalize(DualQuat q)
{
    vec4 qt = q.qT;
    vec4 qr = q.qR;
    float qrLen = sqrt(qr[0]*qr[0] + qr[1]*qr[1] + qr[2]*qr[2] + qr[3]*qr[3]);

    // The len = (1/|q|) is a dual number: a0 + e*a1
    float a0 = 1.f / qrLen;
    float a1 = -dot(qr, qt) / (qrLen * qrLen * qrLen);

    DualQuat result;
    result.qR = a0 * q.qR;
    result.qT = a0 * q.qT + a1 * q.qR;

    return result;
}

DualQuat DualQuatInverse(DualQuat q)
{
    vec4 qt = q.qT;
    vec4 qr = q.qR;
    float qrLen = sqrt(qr[0]*qr[0] + qr[1]*qr[1] + qr[2]*qr[2] + qr[3]*qr[3]);

    // The (1/|q|) is an dual number: a0 + e*a1
    float a0 = 1.f / qrLen;
    float a1 = -dot(qt, qr) / (qrLen * qrLen * qrLen);

    // The (1/|q|)^2 is also an dual number: b0 + e*b1
    float b0 = a0 * a0;
    float b1 = 2 * a0 * a1;

    // The conjugate dual quaternion
    DualQuat cq = q;
    cq.qT[1] = -cq.qT[1];
    cq.qT[2] = -cq.qT[2];
    cq.qT[3] = -cq.qT[3];
    cq.qR[1] = -cq.qR[1];
    cq.qR[2] = -cq.qR[2];
    cq.qR[3] = -cq.qR[3];

    DualQuat result;
    result.qR = b0 * cq.qR;
    result.qT = b0 * cq.qT + b1 * cq.qR;

    return result;
}
DualQuat DLB(float t, DualQuat q1, DualQuat q2)
{
    DualQuat q;
    q.qT = q1.qT * t + q2.qT * (1-t);
    q.qR = q1.qR * t + q2.qR * (1-t);

    return DualQuatNormalize(q);
}
mat4 getTransformationMatrixFromDualQuat(DualQuat q)
{
    float qrLen = sqrt(q.qR[0]*q.qR[0] + q.qR[1]*q.qR[1] + q.qR[2]*q.qR[2] + q.qR[3]*q.qR[3]);

    float tX = 2 * (q.qT[0]*q.qR[1] + q.qT[1]*q.qR[0] - q.qT[2]*q.qR[3] + q.qT[3] * q.qR[2]) / qrLen;
    float tY = 2 * (q.qT[0]*q.qR[2] + q.qT[1]*q.qR[3] - q.qT[2]*q.qR[0] + q.qT[3] * q.qR[1]) / qrLen;
    float tZ = 2 * (q.qT[0]*q.qR[3] + q.qT[1]*q.qR[2] - q.qT[2]*q.qR[1] + q.qT[3] * q.qR[0]) / qrLen;
    mat4 trans = mat4(1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      tX, tY, tZ, 1);

    float w = q.qR[0];
    float x = q.qR[1];
    float y = q.qR[2];
    float z = q.qR[3];
    mat4 rot = mat4(w*w + x*x + y*y - z*z, 2*x*y + 2*w*z, 2*x*z - 2*w*y, 0,
                    2*x*y - 2*w*z, w*w - x*x + y*y - z*z, 2*y*z + 2*w*x, 0,
                    2*x*z + 2*w*y, 2*y*z - 2*w*x, w*w - x*x - y*y + z*z, 0,
                    0, 0, 0, 1);

    return trans * rot;
}



void main()
{
    fs_Col = vs_Col;

    // Method 1: Linear blend
    int id1 = vs_vertInfluenceIDs.x;
    int id2 = vs_vertInfluenceIDs.y;

    float w1 = vs_vertInfluenceWeights[0];
    float w2 = vs_vertInfluenceWeights[1];
    mat4 bindMat1 = u_BindMats[id1];
    mat4 bindMat2 = u_BindMats[id2];
    mat4 tMat1 = u_transformationMats[id1];
    mat4 tMat2 = u_transformationMats[id2];

    mat4 bindMat = w1 * bindMat1 + w2 * bindMat2;
    mat4 tMat = w1 * tMat1 + w2 * tMat2;

    vec4 pos1 = tMat1 * bindMat1 * vs_Pos;
    vec4 pos2 = tMat2 * bindMat2 * vs_Pos;

    vec4 modelposition = u_Model * (pos1 * w1 + pos2 * w2);

    //built-in things to pass down the pipeline
    gl_Position = u_ViewProj * modelposition;



    // Method 2: Dual Quat Blend(can't work...)
//    int id1 = vs_vertInfluenceIDs.x;
//    int id2 = vs_vertInfluenceIDs.y;

//    float w1 = vs_vertInfluenceWeights[0];
//    float w2 = vs_vertInfluenceWeights[1];

//    // Transformation part
//    DualQuat q0, q1;
//    q0.qT = u_transformationDualQuatT[id1];
//    q0.qR = u_transformationDualQuatR[id1];
//    q1.qT = u_transformationDualQuatT[id2];
//    q1.qR = u_transformationDualQuatR[id2];
//    DualQuat q = DLB(w1, q0, q1);
//    mat4 transformation = getTransformationMatrixFromDualQuat(q);

//    // Bind part
//    DualQuat q2, q3;
//    q2.qT = u_dualQuatT[id1];
//    q2.qR = u_dualQuatR[id1];
//    q3.qT = u_dualQuatT[id2];
//    q3.qR = u_dualQuatR[id2];
//    DualQuat qq = DLB(w1, q2, q3);
//    mat4 bindMat = inverse_mat4(getTransformationMatrixFromDualQuat(qq));

//    vec4 vsPos = transformation * bindMat * vs_Pos;

//    vec4 modelposition = u_Model * vsPos;

//    //built-in things to pass down the pipeline
//    gl_Position = u_ViewProj * modelposition;
}
