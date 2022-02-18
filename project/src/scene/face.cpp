#include "face.h"
#include <sstream>

uint Face::s_id = 0;

Face::Face(glm::vec3 color, HalfEdge *HE) :
    m_color(color), mp_HE(HE), m_id(s_id++)
{
    std::stringstream sStr("");
    sStr << "Face_";
    sStr << (m_id);

    setText(sStr.str().c_str());
}
