#ifndef MESH_H
#define MESH_H

#include "drawable.h"
#include "smartpointerhelp.h"
#include "halfedge.h"
#include "skeleton.h"
#include "face.h"
#include "vertex.h"


class MyGL;

class Mesh : public Drawable
{
public:
    Mesh(OpenGLContext *mp_context);

    void reset();

    void createFromObj(const std::vector<glm::vec3> &verts, const std::vector<std::vector<int>> &faces);

    virtual void create() override;

    void createCube();

    void splitEdge(HalfEdge *he);
    void triangulate(Face *face);
    void subdivision();

    void addSkeleton(Skeleton *skeleton);

    friend class MyGL;

signals:
    void sig_addVertsToList(QListWidgetItem *);
    void sig_addFacesToList(QListWidgetItem *);
    void sig_addHalfEdgesToList(QListWidgetItem *);

private:
    bool checkCanTriangulate(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3);
    HalfEdge* findBeforeHE(HalfEdge *he);
    glm::vec4 findFaceNormal(Face *face);

    // Subdivision steps
    void createCentriods();
    void createSmoothMidPoints();
    void smoothOrignalVertices();
    void quadranglate();

    uint findFaceIndex(Face *face);

private:
    // Half edge data
    std::vector<uPtr<Vertex>> m_verts;
    std::vector<uPtr<Face>> m_faces;
    std::vector<uPtr<HalfEdge>> m_HEs;

    // Subdivision data
    uint m_centroidStartIndex;
    uint m_midPointStartIndex;

    // Skin
    Skeleton *m_skeleton;
};

#endif // MESH_H
