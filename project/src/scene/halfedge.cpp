#include "halfedge.h"
#include <sstream>


uint HalfEdge::s_id = 0;

HalfEdge::HalfEdge(HalfEdge *next, HalfEdge *sym, Face *face, Vertex *vert) :
    mp_next(next), mp_sym(sym), mp_face(face), mp_vert(vert), m_id(s_id++)
{
    std::stringstream sStr("");
    sStr << "HE_";
    sStr << (m_id);

    setText(sStr.str().c_str());
}

