#include "mygl.h"
#include <la.h>

#include <iostream>
#include <sstream>
#include <limits>
#include <queue>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <glm/gtx/intersect.hpp>
#include <QApplication>
#include <QKeyEvent>



static inline bool isSpace(const char c) { return (c == ' ') || (c == '\t'); }


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      //m_model(glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0))),
      m_model(glm::mat4(1.f)),
      m_geomSquare(this),
      m_mesh(this),
      m_skeleton(this),
      m_vertDisplay(this),
      m_faceDisplay(this),
      m_halfEdgeDisplay(this),
      m_progLambert(this), m_progFlat(this), m_progSkeleton(this),
      m_glCamera()
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
    m_mesh.destroy();
    m_vertDisplay.destroy();
    m_faceDisplay.destroy();
    m_halfEdgeDisplay.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_mesh.createCube();
    SendMeshNodeToList();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    m_progSkeleton.create(":/glsl/skeleton.vert.glsl", ":/glsl/skeleton.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    m_progSkeleton.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    m_progSkeleton.setViewProjMatrix(m_glCamera.getViewProj());

    //Create a model matrix. This one rotates the square by PI/4 radians then translates it by <-2,0,0>.
    //Note that we have to transpose the model matrix before passing it to the shader
    //This is because OpenGL expects column-major matrices, but you've
    //implemented row-major matrices.
    glm::mat4 model = m_model;

    if(m_mesh.m_skeleton != nullptr)
    {
        m_progSkeleton.setBindMatrics(m_skeleton.getBindMatrics().data());
        m_progSkeleton.setTransformationMatrics(m_skeleton.getTransformationMatrics().data());
        m_progSkeleton.setDualQuatT(m_skeleton.getDualQuatT().data());
        m_progSkeleton.setDualQuatR(m_skeleton.getDualQuatR().data());
        m_progSkeleton.setTransformationDualQuatT(m_skeleton.getTransformationDualQuatT().data());
        m_progSkeleton.setTransformationDualQuatR(m_skeleton.getTransformationDualQuatR().data());

        m_progSkeleton.setModelMatrix(model);
        m_progSkeleton.draw(m_mesh);
    }
    else
    {
        m_progLambert.setModelMatrix(model);
        m_progLambert.draw(m_mesh);
    }

    // Draw displays and skeleton
    {
        glDisable(GL_DEPTH_TEST);

        m_progFlat.setModelMatrix(model);

        if(m_vertDisplay.getDisplay() != nullptr)
        {
            m_progFlat.draw(m_vertDisplay);
        }

        if(m_faceDisplay.getDisplay() != nullptr)
        {
            m_progFlat.draw(m_faceDisplay);
        }

        if(m_halfEdgeDisplay.getDisplay() != nullptr)
        {
            m_progFlat.draw(m_halfEdgeDisplay);
        }

        if(m_skeleton.m_joints.size() > 0)
        {
            m_progFlat.draw(m_skeleton);
        }

        glEnable(GL_DEPTH_TEST);             
    }
}


void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used

    HalfEdge *he = nullptr;
    Vertex *vert = nullptr;
    Face *face = nullptr;
    switch (e->key())
    {
    case Qt::Key_Escape:
        QApplication::quit();
        break;
    case Qt::Key_Right:
        m_glCamera.PolarRotateAlongRight(-amount);
        //m_glCamera.RotateAboutUp(-amount);
        break;
    case Qt::Key_Left:
        m_glCamera.PolarRotateAlongRight(amount);
        //m_glCamera.RotateAboutUp(amount);
        break;
    case Qt::Key_Up:
        m_glCamera.PolarRotateAlongUp(-amount);
        //m_glCamera.RotateAboutRight(-amount);
        break;
    case Qt::Key_Down:
        m_glCamera.PolarRotateAlongUp(amount);
        //m_glCamera.RotateAboutRight(amount);
        break;
    case Qt::Key_1:
        m_glCamera.fovy += amount;
        break;
    case Qt::Key_2:
        m_glCamera.fovy -= amount;
        break;
    case Qt::Key_W:
        m_glCamera.PolarTranslateAlongForward(amount);
        //m_glCamera.TranslateAlongLook(amount);
        break;
    case Qt::Key_S:
        m_glCamera.PolarTranslateAlongForward(-amount);
        //m_glCamera.TranslateAlongLook(-amount);
        break;
    case Qt::Key_D:
        m_glCamera.TranslateAlongRight(amount);
        break;
    case Qt::Key_A:
        m_glCamera.TranslateAlongRight(-amount);
        break;
    case Qt::Key_Q:
        m_glCamera.TranslateAlongUp(-amount);
        break;
    case Qt::Key_E:
        m_glCamera.TranslateAlongUp(amount);
        break;
    case Qt::Key_R:
        m_glCamera = Camera(this->width(), this->height());
        break;
    case Qt::Key_N:
        he = m_halfEdgeDisplay.getDisplay();
        if(he != nullptr)
        {
            m_halfEdgeDisplay.setDisplay(he->mp_next);
            m_halfEdgeDisplay.create();
        }
        break;
    case Qt::Key_M:
        he = m_halfEdgeDisplay.getDisplay();
        if(he != nullptr)
        {
            m_halfEdgeDisplay.setDisplay(he->mp_sym);
            m_halfEdgeDisplay.create();
        }
        break;
    case Qt::Key_F:
        he = m_halfEdgeDisplay.getDisplay();
        if(he != nullptr)
        {
            m_faceDisplay.setDisplay(he->mp_face);
            m_faceDisplay.create();
        }
        break;
    case Qt::Key_V:
         he = m_halfEdgeDisplay.getDisplay();
         if(he != nullptr)
         {
             m_vertDisplay.setDisplay(he->mp_vert);
             m_vertDisplay.create();
         }
         break;
    case Qt::Key_H:
         if(amount == 10.f)
         {
             face = m_faceDisplay.getDisplay();
             if(face != nullptr)
             {
                m_halfEdgeDisplay.setDisplay(face->mp_HE);
                m_halfEdgeDisplay.create();
             }
         }
         else
         {
             vert = m_vertDisplay.getDisplay();
             if(vert != nullptr)
             {
                 m_halfEdgeDisplay.setDisplay(vert->mp_HE);
                 m_halfEdgeDisplay.create();
             }
         }
         break;

    case Qt::Key_0:
        m_glCamera.PolarRotateAlongUp(5);
        break;
    default:
        break;
    }

    m_glCamera.RecomputeAttributes();

    update();  // Calls paintGL, among other things
}

void MyGL::SendMeshNodeToList()
{
    emit sig_clearVertsList();
    emit sig_clearFacesList();
    emit sig_clearHalfEdgesList();

   for(uint i = 0; i < m_mesh.m_verts.size(); i++)
   {
       emit sig_addVertsToList(m_mesh.m_verts[i].get());
   }

   for(uint i = 0; i < m_mesh.m_faces.size(); i++)
   {
       emit sig_addFacesToList(m_mesh.m_faces[i].get());
   }

   for(uint i = 0; i < m_mesh.m_HEs.size(); i++)
   {
       emit sig_addHalfEdgesToList(m_mesh.m_HEs[i].get());
   }
}

void MyGL::slot_onVertexSelected(QListWidgetItem *item)
{
    // Draw displays
    Vertex *vert = dynamic_cast<Vertex*>(item);
    m_vertDisplay.setDisplay(vert);
    m_vertDisplay.create();

    emit sig_refreshVertPosX(vert->m_pos.x);
    emit sig_refreshVertPosY(vert->m_pos.y);
    emit sig_refreshVertPosZ(vert->m_pos.z);

    update();

    // Send refresh signal

}
void MyGL::slot_onFaceSelected(QListWidgetItem *item)
{
    Face *face = dynamic_cast<Face*>(item);
    m_faceDisplay.setDisplay(face);
    m_faceDisplay.create();

    emit sig_refreshFaceRed(face->m_color.r);
    emit sig_refreshFaceGreen(face->m_color.g);
    emit sig_refreshFaceBlue(face->m_color.b);

    update();
}
void MyGL::slot_onHalfEdgeSelected(QListWidgetItem *item)
{
    HalfEdge *he = dynamic_cast<HalfEdge*>(item);
    m_halfEdgeDisplay.setDisplay(he);
    m_halfEdgeDisplay.create();

    update();
}

void MyGL::slot_onJointSelected(QTreeWidgetItem *item, int)
{
    Joint *joint = dynamic_cast<Joint*>(item);

    m_skeleton.selectedJoint = joint;
    m_skeleton.create();

    emit sig_refreshJointPosX(joint->m_pos[0]);
    emit sig_refreshJointPosY(joint->m_pos[1]);
    emit sig_refreshJointPosZ(joint->m_pos[2]);

    update();
}

void MyGL::on_vertPosXChanged(double d)
{
    Vertex *vert = m_vertDisplay.getDisplay();
    if(vert == nullptr)
    {
        return;
    }

    vert->m_pos = glm::vec3(d, vert->m_pos.y, vert->m_pos.z);
    m_vertDisplay.create();

    m_mesh.create();

    update();
}
void MyGL::on_vertPosYChanged(double d)
{
    Vertex *vert = m_vertDisplay.getDisplay();
    if(vert == nullptr)
    {
        return;
    }

    vert->m_pos = glm::vec3(vert->m_pos.x, d, vert->m_pos.z);
    m_vertDisplay.create();

    m_mesh.create();

    update();
}
void MyGL::on_vertPosZChanged(double d)
{
    Vertex *vert = m_vertDisplay.getDisplay();
    if(vert == nullptr)
    {
        return;
    }

    vert->m_pos = glm::vec3(vert->m_pos.x, vert->m_pos.y, d);
    m_vertDisplay.create();

    m_mesh.create();

    update();
}

void MyGL::on_faceRedChanged(double d)
{
    Face *face = m_faceDisplay.getDisplay();
    if(face == nullptr)
    {
        return;
    }

    face->m_color = glm::vec3(d, face->m_color.g, face->m_color.b);
    m_faceDisplay.create();

    m_mesh.create();

    update();
}
void MyGL::on_faceGreenChanged(double d)
{
    Face *face = m_faceDisplay.getDisplay();
    if(face == nullptr)
    {
        return;
    }

    face->m_color = glm::vec3(face->m_color.r, d, face->m_color.b);
    m_faceDisplay.create();

    m_mesh.create();

    update();
}
void MyGL::on_faceBlueChanged(double d)
{
    Face *face = m_faceDisplay.getDisplay();
    if(face == nullptr)
    {
        return;
    }

    face->m_color = glm::vec3(face->m_color.r, face->m_color.g, d);
    m_faceDisplay.create();

    m_mesh.create();

    update();
}

void MyGL::on_jointPosXChanged(double d)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    glm::vec3 pos = m_skeleton.selectedJoint->m_pos;
    pos.x = d;
    m_skeleton.selectedJoint->setPosition(pos);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointPosYChanged(double d)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    glm::vec3 pos = m_skeleton.selectedJoint->m_pos;
    pos.y = d;
    m_skeleton.selectedJoint->setPosition(pos);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointPosZChanged(double d)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    glm::vec3 pos = m_skeleton.selectedJoint->m_pos;
    pos.z = d;
    m_skeleton.selectedJoint->setPosition(pos);
    m_skeleton.create();

    m_mesh.create();

    update();
}

void MyGL::on_jointRotX(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(1, 0, 0), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointRotMinusX(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(-1, 0, 0), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointRotY(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(0, 1, 0), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointRotMinusY(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(0, -1, 0), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointRotZ(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(0, 0, 1), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}
void MyGL::on_jointRotMinusZ(bool)
{
    if(m_skeleton.selectedJoint == nullptr)
    {
        return;
    }

    m_skeleton.selectedJoint->rotate(glm::vec3(0, 0, -1), 5);
    m_skeleton.create();

    m_mesh.create();

    update();
}

void MyGL::on_SplitEdge(bool)
{
    HalfEdge *he = m_halfEdgeDisplay.getDisplay();

    if(he != nullptr)
    {
        m_mesh.splitEdge(he);

        m_mesh.create();

        emit sig_addVertsToList(m_mesh.m_verts[m_mesh.m_verts.size() - 1].get());
        emit sig_addHalfEdgesToList(m_mesh.m_HEs[m_mesh.m_HEs.size() - 2].get());
        emit sig_addHalfEdgesToList(m_mesh.m_HEs[m_mesh.m_HEs.size() - 1].get());

        update();
    }
}

void MyGL::on_Triangulate(bool)
{
    Face *face = m_faceDisplay.getDisplay();

    if(face != nullptr)
    {
        int faceCountBefore = m_mesh.m_faces.size();
        int heCountBefore = m_mesh.m_HEs.size();

        m_mesh.triangulate(face);

        for(uint i = faceCountBefore; i < m_mesh.m_faces.size(); i++)
        {
            emit sig_addFacesToList(m_mesh.m_faces[i].get());
        }

        for(uint i = heCountBefore; i < m_mesh.m_HEs.size(); i++)
        {
            emit sig_addHalfEdgesToList(m_mesh.m_HEs[i].get());
        }

        m_mesh.create();
    }
}

void MyGL::on_Subdivision(bool)
{
    uint vertCount = m_mesh.m_verts.size();
    uint HECount = m_mesh.m_HEs.size();
    uint faceCount = m_mesh.m_faces.size();

    m_mesh.subdivision();
    m_mesh.create();

    for(uint i = vertCount; i < m_mesh.m_verts.size(); i++)
    {
        emit sig_addVertsToList(m_mesh.m_verts[i].get());
    }
    for(uint i = HECount; i < m_mesh.m_HEs.size(); i++)
    {
        emit sig_addHalfEdgesToList(m_mesh.m_HEs[i].get());
    }
    for(uint i = faceCount; i < m_mesh.m_faces.size(); i++)
    {
        emit sig_addFacesToList(m_mesh.m_faces[i].get());
    }

    update();
}

void MyGL::on_LoadObj(bool)
{
    QString filename = QFileDialog::getOpenFileName(0, QString("Load Scene File"), QDir::currentPath().append(QString("../..")), QString("*.obj"));

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file [" << filename.toStdString() << "]" << std::endl;
        return;
    }


    std::vector<glm::vec3> verts;
    std::vector<std::vector<int>> faces;

    QTextStream ifs(&file);
    QString buf;
    while(!ifs.atEnd())
    {
        buf = ifs.readLine();

        std::string linebuf(buf.toStdString());

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0)
        {
          if (linebuf[linebuf.size() - 1] == '\n')
            linebuf.erase(linebuf.size() - 1);
        }
        if (linebuf.size() > 0)
        {
          if (linebuf[linebuf.size() - 1] == '\r')
            linebuf.erase(linebuf.size() - 1);
        }

        // Skip if empty line.
        if (linebuf.empty())
        {
            continue;
        }

        // Check if line can be prased
        std::istringstream iss(linebuf);
        if(!iss)
        {
            continue;
        }

        // Get vertex and token
        std::string token;
        iss >> token;
        if(token == "v")
        {
            float f1, f2, f3;
            iss >> f1;
            iss >> f2;
            iss >> f3;

            verts.push_back(glm::vec3(f1, f2, f3));
        }
        else if(token == "f")
        {
            std::vector<int> face;
            std::string str;
            while(iss >> str)
            {
                face.push_back(std::atoi(str.substr(0, str.find('/')).c_str()) - 1);
            }
            faces.push_back(face);
        }
    }

    m_mesh.createFromObj(verts, faces);
    m_mesh.create();

    SendMeshNodeToList();
}

void MyGL::on_LoadJson(bool)
{
    QString filename = QFileDialog::getOpenFileName(0, QString("Load Json File"),
                                                    QDir::currentPath().append(QString("../..")), QString("*.json"));

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        std::cerr << "Could not open file [" << filename.toStdString() << "]" << std::endl;
        return;
    }

    QByteArray data = file.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data, &jsonError));
    if(jsonError.error != QJsonParseError::NoError)
    {
        std::cerr << "json error!" << jsonError.errorString().toStdString() << std::endl;
        return;
    }

    std::vector<std::string> namesVec;
    std::vector<glm::vec3> posVec;
    std::vector<glm::quat> rotsVec;
    std::map<std::string, std::string> map; // Map of joint and its parent

    // Parse json in depth-first order
    std::queue<QJsonObject> jsonQueue;
    jsonQueue.push(jsonDoc.object().value("root").toObject());
    while(!jsonQueue.empty())
    {
        QJsonObject jsonObj = jsonQueue.front();
        jsonQueue.pop();

        if(jsonObj.contains("name"))
        {
            QJsonValue value = jsonObj.value("name");
            if(value.isString())
            {
                namesVec.push_back(value.toString().toStdString());
            }
        }

        if(jsonObj.contains("pos"))
        {
            QJsonValue value = jsonObj.value("pos");
            if(value.isArray())
            {
                glm::vec3 pos;

                QJsonArray array = value.toArray();
                for(int i = 0; i < 3; i++)
                {
                    if(array.at(i).isDouble())
                    {
                        pos[i] = array.at(i).toDouble();
                    }
                }

                posVec.push_back(pos);
            }
        }

        if(jsonObj.contains("rot"))
        {
            QJsonValue value = jsonObj.value("rot");
            if(value.isArray())
            {
                double angle;
                glm::vec3 axis;

                QJsonArray array = value.toArray();
                for(int i = 0; i < 4; i++)
                {
                    if(array.at(i).isDouble())
                    {
                        if(i == 0)
                        {
                            angle = array.at(i).toDouble();
                        }
                        else
                        {
                            axis[i - 1] = array.at(i).toDouble();
                        }
                    }
                }

                rotsVec.push_back(glm::angleAxis((float)angle, axis));
            }
        }

        if(jsonObj.contains("children"))
        {
            QJsonValue value = jsonObj.value("children");
            if(value.isArray())
            {
                QJsonArray array = value.toArray();
                if(array.size() == 0)
                {
                    continue;
                }

                for(int i = 0; i < array.size(); i++)
                {
                    QJsonValue subValue = array.at(i);
                    if(subValue.isObject())
                    {
                        QJsonObject childObj = subValue.toObject();

                        // Set parent for each child
                        if(childObj.contains("name"))
                        {
                            QJsonValue childNameValue = childObj.value("name");
                            if(childNameValue.isString())
                            {
                                map.insert(std::pair<std::string, std::string>(childNameValue.toString().toStdString(), namesVec.back()));
                            }
                        }

                        jsonQueue.push(subValue.toObject());
                    }
                }
            }
        }
    }

    m_skeleton.createJoints(namesVec, posVec, rotsVec, map);
    m_skeleton.create();

    emit sig_addJointsToTree(m_skeleton.m_joints[0].get());

    update();
}

void MyGL::on_SkinMesh(bool)
{
    m_mesh.addSkeleton(&m_skeleton);

    m_mesh.create();

    update();
}

void MyGL::mousePressEvent(QMouseEvent *event)
{
    // 1. Create ray
    QPoint point = event->pos();
    glm::vec4 pos = glm::vec4(point.x(), point.y(), 1, 1);

    // Pixel space to screen space
    pos.x = (pos.x / this->width()) * 2 - 1;
    pos.y = 1 - (pos.y / this->height()) * 2;
    pos = pos * m_glCamera.far_clip;

    // Screen space to world space
    glm::mat4 proj_inv = glm::inverse(m_glCamera.getProjMat());
    glm::mat4 view_inv = glm::inverse(m_glCamera.getViewMat());
    pos = view_inv * proj_inv * pos;

    glm::vec3 eyePos = m_glCamera.eye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(pos) - eyePos);

    glm::mat4 model_inv = glm::inverse(m_model);
    rayDir = glm::vec3(model_inv * glm::vec4(rayDir, 0));
    eyePos = glm::vec3(model_inv * glm::vec4(eyePos, 1));

    // 2. Cast ray to each vertex(treat as sphere)
    float closestDist = INFINITY;
    Vertex *closestVert = nullptr;
    for(uint i = 0; i < m_mesh.m_verts.size(); i++)
    {
        Vertex *vert = m_mesh.m_verts[i].get();

        glm::vec3 intersectPos = glm::vec3();
        glm::vec3 intersectNor = glm::vec3();
        if(glm::intersectRaySphere(eyePos, rayDir, vert->m_pos, 0.1f, intersectPos, intersectNor))
        {
            float distance = glm::distance(intersectPos, eyePos);
            if(distance < closestDist)
            {
                closestDist = distance;
                closestVert = vert;
            }
        }
    }
    if(closestVert != nullptr)
    {
        m_vertDisplay.setDisplay(closestVert);
        m_vertDisplay.create();

        update();
    }
}

