#include "vertexdisplay.h"

VertexDisplay::VertexDisplay(OpenGLContext *context) :
    Drawable(context), m_vert(nullptr)
{}

void VertexDisplay::create()
{
    glm::vec4 pos[] = {glm::vec4(m_vert->m_pos, 1)};
    glm::vec4 col[] = {glm::vec4(1, 1, 1, 1)};
    GLuint id[] = {0};

    count = 1;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint), id, GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), pos, GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

void VertexDisplay::setDisplay(Vertex *v)
{
    m_vert = v;
}

Vertex* VertexDisplay::getDisplay()
{
    return m_vert;
}

GLenum VertexDisplay::drawMode()
{
    return GL_POINTS;
}
