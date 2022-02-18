#ifndef FACE_H
#define FACE_H


#include <la.h>
#include <QListWidgetItem>

class HalfEdge;

class Face : public QListWidgetItem
{
public:
    Face(glm::vec3 color, HalfEdge *HE);

public:
    glm::vec3 m_color;
    HalfEdge *mp_HE;

    uint m_id;

public:
    static uint s_id;
};

#endif // FACE_H
