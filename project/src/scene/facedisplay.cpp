#include "facedisplay.h"
#include "halfedge.h"
#include "vertex.h"


FaceDisplay::FaceDisplay(OpenGLContext *context) :
    Drawable(context), m_face(nullptr)
{}

void FaceDisplay::setDisplay(Face *face)
{
    m_face = face;
}

Face* FaceDisplay::getDisplay()
{
    return m_face;
}

void FaceDisplay::create()
{
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;

    glm::vec4 color = glm::vec4(1 - m_face->m_color.r, 1 - m_face->m_color.g, 1 - m_face->m_color.b, 1);
    HalfEdge *he = m_face->mp_HE;
    HalfEdge *start = he;
    int num = 0;
    do
    {
        pos.push_back(glm::vec4(he->mp_vert->m_pos, 1));
        pos.push_back(glm::vec4(he->mp_next->mp_vert->m_pos, 1));
        col.push_back(color);
        col.push_back(color);
        idx.push_back(num++);
        idx.push_back(num++);

        he = he->mp_next;
    }
    while(he != start);

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

GLenum FaceDisplay::drawMode()
{
    return GL_LINES;
}

