#ifndef FACEDISPLAY_H
#define FACEDISPLAY_H


#include "drawable.h"
#include "face.h"

class FaceDisplay : public Drawable
{
public:
    FaceDisplay(OpenGLContext *context);

    void setDisplay(Face *face);

    Face* getDisplay();

    virtual GLenum drawMode() override;
    virtual void create() override;

private:
    Face *m_face;
};

#endif // FACEDISPLAY_H
