#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <la.h>
#include <QListWidgetItem>

class Face;
class Vertex;

class HalfEdge : public QListWidgetItem
{
public:
    HalfEdge(HalfEdge *next, HalfEdge *sym, Face *face, Vertex *vert);

public:
    HalfEdge *mp_next;  // Pointer to next half edge
    HalfEdge *mp_sym;   // Pointer to the symmetrical half edge
    Face *mp_face;
    Vertex *mp_vert;

    uint m_id;

public:
    static uint s_id;
};

#endif // HALFEDGE_H
