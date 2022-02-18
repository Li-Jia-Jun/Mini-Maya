#include "skeleton.h"


Skeleton::Skeleton(OpenGLContext *mp_context) :
    Drawable(mp_context)
{}

void Skeleton::create()
{
    glm::vec4 magentaCol = glm::vec4(1, 0, 1, 1);
    glm::vec4 yellowCol = glm::vec4(1, 1, 0, 1);
    glm::vec4 redCol = glm::vec4(1, 0, 0, 1);
    glm::vec4 greenCol = glm::vec4(0, 1, 0, 1);
    glm::vec4 blueCol = glm::vec4(0, 0, 1, 1);
    glm::vec4 selectCol = glm::vec4(0.9, 0.9, 0.9, 1);

    float twoPi = glm::pi<float>() * 2;
    float radius = 0.5f;
    int N = 10;


    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;

    for(auto &j : m_joints)
    {
        Joint *joint = j.get();

        // Line between each joint
        for(uint i = 0; i < joint->m_children.size(); i++)
        {
            pos.push_back(joint->getWorldPosition());
            pos.push_back(joint->m_children[i]->getWorldPosition());
            col.push_back(magentaCol);
            col.push_back(yellowCol);
        }

        // Wireframe sphere of each joint
        glm::mat4 jMat = joint->getOverallTransformation();
        bool isSelected = selectedJoint != nullptr && selectedJoint->m_id == joint->m_id;
        for(int i = 0; i < N; i++)
        {
            float f1 = ((float)i / N) * twoPi;
            float f2 = ((float)(i + 1) / N) * twoPi;

            float s1 = glm::sin(f1) * radius;
            float c1 = glm::cos(f1) * radius;
            float s2 = glm::sin(f2) * radius;
            float c2 = glm::cos(f2) * radius;

            // X-Z plane circle
            glm::vec4 xz1 = jMat * glm::vec4(s1, 0, c1, 1);
            glm::vec4 xz2 = jMat * glm::vec4(s2, 0, c2, 1);
            pos.push_back(xz1);
            pos.push_back(xz2);
            col.push_back(isSelected ? selectCol : greenCol);
            col.push_back(isSelected ? selectCol : greenCol);

            // X-Y plane circle
            glm::vec4 xy1 = jMat * glm::vec4(s1, c1, 0, 1);
            glm::vec4 xy2 = jMat * glm::vec4(s2, c2, 0, 1);
            pos.push_back(xy1);
            pos.push_back(xy2);
            col.push_back(isSelected ? selectCol : blueCol);
            col.push_back(isSelected ? selectCol : blueCol);

            // Y-Z plane circle
            glm::vec4 yz1 = jMat * glm::vec4(0, s1, c1, 1);
            glm::vec4 yz2 = jMat * glm::vec4(0, s2, c2, 1);
            pos.push_back(yz1);
            pos.push_back(yz2);
            col.push_back(isSelected ? selectCol : redCol);
            col.push_back(isSelected ? selectCol : redCol);
        }
    }

    for(uint i = 0; i < pos.size(); i++)
    {
        idx.push_back(i);
    }


    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum Skeleton::drawMode()
{
    return GL_LINES;
}

void Skeleton::reset()
{
    m_joints.clear();
    Joint::s_id = 0;
}

void Skeleton::createJoints(std::vector<std::string>& names, std::vector<glm::vec3>& pos, std::vector<glm::quat> rots,
                        std::map<std::string, std::string> map)
{
    reset();

    // Create joints
    for(uint i = 0; i < names.size(); i++)
    {
        m_joints.push_back(mkU<Joint>(names[i], nullptr, pos[i], rots[i]));
    }

    // Set parent and children
    for(auto &pair : map)
    {
        Joint *j1 = getJointByName(pair.first);
        Joint *j2 = getJointByName(pair.second);
        j1->setJointParent(j2);
        j2->addJointChild(j1);
    }

    for(auto &j : m_joints)
    {
        j->computeBindMatrix();
    }
}

Joint* Skeleton::getJointByName(std::string name)
{
    for(auto &j : m_joints)
    {
        if(j->m_name == name)
        {
            return j.get();
        }
    }

    return nullptr;
}

std::vector<glm::mat4> Skeleton::getBindMatrics()
{
    std::vector<glm::mat4> mats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        mats[i] = m_joints[i]->m_bindMat;
    }

    return mats;
}
std::vector<glm::mat4> Skeleton::getTransformationMatrics()
{
    std::vector<glm::mat4> mats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        mats[i] = m_joints[i]->getOverallTransformation();
    }

    return mats;
}
std::vector<glm::quat> Skeleton::getDualQuatT()
{
    std::vector<glm::quat> quats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        quats[i] = m_joints[i]->m_bindDualQuatT;
    }

    return quats;
}
std::vector<glm::quat> Skeleton::getDualQuatR()
{
    std::vector<glm::quat> quats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        quats[i] = m_joints[i]->m_bindDualQuatR;
    }

    return quats;
}
std::vector<glm::quat> Skeleton::getTransformationDualQuatT()
{
    std::vector<glm::quat> quats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        quats[i] = m_joints[i]->getDualQuatT();
    }

    return quats;
}
std::vector<glm::quat> Skeleton::getTransformationDualQuatR()
{
    std::vector<glm::quat> quats(100);

    for(uint i = 0; i < m_joints.size(); i++)
    {
        quats[i] = m_joints[i]->getDualQuatR();
    }

    return quats;
}
