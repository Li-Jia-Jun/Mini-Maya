#ifndef VERTEXDISPLAY_H
#define VERTEXDISPLAY_H


#include "drawable.h"
#include "vertex.h"

class VertexDisplay : public Drawable
{
public:    
    VertexDisplay(OpenGLContext *context);

    void setDisplay(Vertex *v);

    Vertex* getDisplay();

    virtual void create() override;

    virtual GLenum drawMode() override;

private:
    Vertex *m_vert;
};

#endif // VERTEXDISPLAY_H
