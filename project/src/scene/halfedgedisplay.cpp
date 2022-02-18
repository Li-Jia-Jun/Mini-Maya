#include "halfedgedisplay.h"
#include "vertex.h"


HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext *context) :
    Drawable(context), m_HE(nullptr)
{}

void HalfEdgeDisplay::setDisplay(HalfEdge *he)
{
    m_HE = he;
}

HalfEdge* HalfEdgeDisplay::getDisplay()
{
    return m_HE;
}

GLenum HalfEdgeDisplay::drawMode()
{
    return GL_LINES;
}


void HalfEdgeDisplay::create()
{
    glm::vec4 pos[] = {glm::vec4(m_HE->mp_vert->m_pos, 1), glm::vec4()};
    glm::vec4 col[] = {glm::vec4(1, 1, 0, 1), glm::vec4(1, 0, 0, 1)};
    GLuint id[] = {0, 1};

    HalfEdge *p = m_HE;
    while(p->mp_next != m_HE)
    {
        p = p->mp_next;
    }
    pos[1] = glm::vec4(p->mp_vert->m_pos, 1);

    count = 2;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(GLuint), id, GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}
