#ifndef VERTEX_H
#define VERTEX_H

#include <la.h>
#include <QListWidgetItem>
#include <ostream>

class HalfEdge;

class Vertex : public QListWidgetItem
{
public:
    Vertex(glm::vec3 pos, HalfEdge *HE);

    void setJointInfluence(std::vector<uint> jointIDs, std::vector<float> influences);

public:
    glm::vec3 m_pos;
    HalfEdge *mp_HE;

    uint m_id;

    std::vector<uint> mp_jointIDs;
    std::vector<float> m_jointInfluences;

public:
    static uint s_id;
};

std::ostream& operator<<(std::ostream &o, const Vertex& vert);

#endif // VERTEX_H
