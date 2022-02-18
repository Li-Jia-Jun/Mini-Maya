#include "joint.h"

uint Joint::s_id = 0;

Joint::Joint(std::string name, Joint *parent, glm::vec3 pos, glm::quat rot) :
    m_name(name), mp_parent(parent), m_pos(pos), m_rot(rot), m_id(s_id++)
{
    setText(0, name.c_str());
}

void Joint::setPosition(glm::vec3 t)
{
    m_pos = t;
}

void Joint::rotate(glm::vec3 axis, float angle)
{
    glm::quat q = glm::angleAxis(glm::radians(angle), axis);
    m_rot = q * m_rot;  // Pre-multiply since rotate about world axis
}

void Joint::setJointParent(Joint *parent)
{
    mp_parent = parent;
}

void Joint::addJointChild(Joint* child)
{
    m_children.push_back(child);
    addChild(child);
}

glm::mat4 Joint::getLocalTransformation()
{
    glm::mat4 trans = glm::mat4(1.f);
    trans = glm::translate(trans, m_pos);

    glm::mat4 rot = glm::mat4_cast(m_rot);

    return trans * rot;
}

glm::mat4 Joint::getOverallTransformation()
{
    glm::mat4 localMat = getLocalTransformation();
    glm::mat4 parentMat = mp_parent != nullptr ? mp_parent->getOverallTransformation() : glm::mat4(1.f);

    return parentMat * localMat;
}

glm::vec4 Joint::getWorldPosition()
{
    return getOverallTransformation() * glm::vec4(0, 0, 0, 1);
}

glm::quat Joint::getDualQuatT()
{
    glm::vec4 worldPos = getOverallTransformation() * glm::vec4(0, 0, 0, 1);
    glm::quat worldRot = glm::quat(getOverallTransformation());

    float w = 0.5f * (worldPos[0] * worldRot.x - worldPos[1] * worldRot.y - worldPos[2] * worldRot.z);
    float i = 0.5f * (worldPos[0] * worldRot.w + worldPos[1] * worldRot.z - worldPos[2] * worldRot.y);
    float j = 0.5f * (-worldPos[0] * worldRot.z + worldPos[1] * worldRot.w + worldPos[2] * worldRot.x);
    float k = 0.5f * (worldPos[0] * worldRot.y - worldPos[1] * worldRot.x + worldPos[2] * worldRot.w);

    return glm::quat(w, i, j, k);
}

glm::quat Joint::getDualQuatR()
{
    // Get the rotation part of the whole transformation
    glm::mat4 mat = getOverallTransformation();
    for(int i = 0; i < 3; i++)
    {
        mat[3][i] = 0;
        mat[i][3] = 0;
    }

    glm::quat ret = glm::toQuat(mat);
    return ret;

}

void Joint::computeBindMatrix()
{
    m_bindMat = glm::inverse(getOverallTransformation());

    m_bindDualQuatT = getDualQuatT();
    m_bindDualQuatR = getDualQuatR();
}
