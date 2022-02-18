#ifndef JOINT_H
#define JOINT_H


#include <la.h>
#include <QTreeWidgetItem>

class Joint : public QTreeWidgetItem
{
public:
    Joint(std::string name,  Joint *parent, glm::vec3 pos, glm::quat rot);

    void setPosition(glm::vec3 t);
    void rotate(glm::vec3 axis, float angle);

    void setJointParent(Joint *parent);
    void addJointChild(Joint* child);

    glm::mat4 getLocalTransformation();
    glm::mat4 getOverallTransformation();

    glm::vec4 getWorldPosition();

    glm::quat getDualQuatT();
    glm::quat getDualQuatR();

public:
    void computeBindMatrix();

public:
    std::string m_name;

    Joint *mp_parent;
    std::vector<Joint*> m_children;

    glm::vec3 m_pos;        // With respect to parent's joint
    glm::quat m_rot;        // With respect to parent's joint
    glm::mat4 m_bindMat;

    glm::quat m_bindDualQuatT;
    glm::quat m_bindDualQuatR;

    uint m_id;
    static uint s_id;
};

#endif // JOINT_H
