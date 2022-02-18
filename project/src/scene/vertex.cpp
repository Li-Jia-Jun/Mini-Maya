#include "vertex.h"
#include <sstream>


uint Vertex::s_id = 0;

Vertex::Vertex(glm::vec3 pos, HalfEdge * HE) :
    m_pos(pos), mp_HE(HE), m_id(s_id++)
{
    std::stringstream sStr("");
    sStr << "Vert_";
    sStr << (m_id);

    setText(sStr.str().c_str());
}

void Vertex::setJointInfluence(std::vector<uint> jointIDs, std::vector<float> influences)
{
    mp_jointIDs.clear();
    m_jointInfluences.clear();

    mp_jointIDs.insert(mp_jointIDs.end(), jointIDs.begin(), jointIDs.end());
    m_jointInfluences.insert(m_jointInfluences.end(), influences.begin(), influences.end());
}

std::ostream& operator<<(std::ostream &o, const Vertex& vert)
{
    o << "v" << vert.m_id << "  ";
    o << "(" << vert.m_pos.x << ", " << vert.m_pos.y << ", " << vert.m_pos.z << ")";
    return o;
}

