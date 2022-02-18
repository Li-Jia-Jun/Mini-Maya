#ifndef SKELETON_H
#define SKELETON_H


#include "smartpointerhelp.h"
#include "la.h"
#include "drawable.h"
#include "joint.h"

class MyGL;
class Mesh;

class Skeleton : public Drawable
{
public:
    Skeleton(OpenGLContext *mp_context);

    void createJoints(std::vector<std::string>& names, std::vector<glm::vec3>& pos, std::vector<glm::quat> rots,
                      std::map<std::string, std::string> map);

    void reset();

    std::vector<glm::mat4> getBindMatrics();
    std::vector<glm::mat4> getTransformationMatrics();
    std::vector<glm::quat> getDualQuatT();
    std::vector<glm::quat> getDualQuatR();
    std::vector<glm::quat> getTransformationDualQuatT();
    std::vector<glm::quat> getTransformationDualQuatR();

    virtual void create() override;
    virtual GLenum drawMode() override;

    friend class MyGL;
    friend class Mesh;

private:
    Joint* getJointByName(std::string name);

private:
    std::vector<uPtr<Joint>> m_joints;

    Joint *selectedJoint;
};

#endif // SKELETON_H
