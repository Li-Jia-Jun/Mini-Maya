#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"
#include "scene/mesh.h"
#include "scene/skeleton.h"
#include "scene/vertexdisplay.h"
#include "scene/facedisplay.h"
#include "scene/halfedgedisplay.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    Mesh m_mesh;
    Skeleton m_skeleton;

    VertexDisplay m_vertDisplay;
    FaceDisplay m_faceDisplay;
    HalfEdgeDisplay m_halfEdgeDisplay;

    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progSkeleton;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

    glm::mat4 m_model;

private:
    void addMeshToWigetList();

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *e);
    void SendMeshNodeToList();

signals:
    void sig_addVertsToList(QListWidgetItem *);
    void sig_addFacesToList(QListWidgetItem *);
    void sig_addHalfEdgesToList(QListWidgetItem *);

    void sig_refreshVertPosX(double);
    void sig_refreshVertPosY(double);
    void sig_refreshVertPosZ(double);

    void sig_refreshFaceRed(double);
    void sig_refreshFaceGreen(double);
    void sig_refreshFaceBlue(double);

    void sig_refreshJointPosX(double);
    void sig_refreshJointPosY(double);
    void sig_refreshJointPosZ(double);

    void sig_clearVertsList();
    void sig_clearFacesList();
    void sig_clearHalfEdgesList();

    void sig_addJointsToTree(QTreeWidgetItem *);

private slots:
    void slot_onVertexSelected(QListWidgetItem *);
    void slot_onFaceSelected(QListWidgetItem *);
    void slot_onHalfEdgeSelected(QListWidgetItem *);

    void slot_onJointSelected(QTreeWidgetItem *, int);

    void on_vertPosXChanged(double);
    void on_vertPosYChanged(double);
    void on_vertPosZChanged(double);

    void on_faceRedChanged(double);
    void on_faceGreenChanged(double);
    void on_faceBlueChanged(double);

    void on_jointPosXChanged(double);
    void on_jointPosYChanged(double);
    void on_jointPosZChanged(double);

    void on_jointRotX(bool);
    void on_jointRotMinusX(bool);
    void on_jointRotY(bool);
    void on_jointRotMinusY(bool);
    void on_jointRotZ(bool);
    void on_jointRotMinusZ(bool);

    void on_SplitEdge(bool);
    void on_Triangulate(bool);
    void on_Subdivision(bool);   
    void on_LoadObj(bool);
    void on_LoadJson(bool);
    void on_SkinMesh(bool);
};

#endif // MYGL_H
