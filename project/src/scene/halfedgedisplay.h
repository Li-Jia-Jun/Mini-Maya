#ifndef HALFEDGEDISPLAY_H
#define HALFEDGEDISPLAY_H


#include "halfedge.h"
#include "drawable.h"


class HalfEdgeDisplay : public Drawable
{
public:
    HalfEdgeDisplay(OpenGLContext *context);

    void setDisplay(HalfEdge *he);

    HalfEdge* getDisplay();

    virtual GLenum drawMode() override;
    virtual void create() override;

private:
    HalfEdge *m_HE;
};

#endif // HALFEDGEDISPLAY_H
