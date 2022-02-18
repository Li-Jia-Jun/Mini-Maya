#include "mesh.h"
#include <map>
#include <unordered_set>
#include <iostream>
#include <glm/gtx/random.hpp>

int _cubeVertIdxRef[6][4]
{
    {1, 2, 3, 0},   // Front face
    {1, 5, 6, 2},   // Right face
    {7, 6, 5, 4},   // Back face
    {4, 0, 3, 7},   // Left face
    {3, 2, 6, 7},   // Up face
    {4, 5, 1, 0}    // Down face
};
glm::vec3 _cubeFaceColor = glm::vec3(0.2, 0.3, 0.5);


Mesh::Mesh(OpenGLContext *mp_context) :
    Drawable(mp_context)
{}

void Mesh::reset()
{
    Vertex::s_id = 0;
    Face::s_id = 0;
    HalfEdge::s_id = 0;

    m_verts.clear();
    m_faces.clear();
    m_HEs.clear();

    m_skeleton = nullptr;
}

void Mesh::create()
{
    std::vector<glm::vec4> posVec;
    std::vector<glm::vec4> norVec;
    std::vector<glm::vec4> colVec;
    std::vector<glm::ivec2> jointIDs;
    std::vector<glm::vec2> jointWeights;
    std::vector<GLuint> idx;

    // Generate data face by face
    int size = m_faces.size();
    for(int i = 0; i < size; i++)
    {
        int posAdded = posVec.size();

        HalfEdge *st = m_faces[i].get()->mp_HE;
        HalfEdge *he = st;

        // Find face normal
        glm::vec4 nor = findFaceNormal(m_faces[i].get());

        // Add position, normal and color VBO
        he = st;
        do
        {
            posVec.push_back(glm::vec4(he->mp_vert->m_pos, 1));
            norVec.push_back(nor);
            colVec.push_back(glm::vec4(m_faces[i].get()->m_color, 1));

            if(m_skeleton != nullptr)
            {                
                jointIDs.push_back(glm::ivec2(he->mp_vert->mp_jointIDs[0], he->mp_vert->mp_jointIDs[1]));
                jointWeights.push_back(glm::vec2(he->mp_vert->m_jointInfluences[0], he->mp_vert->m_jointInfluences[1]));
            }

            he = he->mp_next;
        }
        while(he != st);

        // Add vertex indices
        int thisAdd = norVec.size() - posAdded;
        for(int j = 0; j < thisAdd - 2; j++)
        {
            idx.push_back(posAdded);
            idx.push_back(posAdded + j + 1);
            idx.push_back(posAdded + j + 2);
        }
    }


    // Pass data
    count = idx.size();

    if(m_skeleton != nullptr)
    {
        generateVertInfluenceIDs();                
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufVertInfluenceIDs);       
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, jointIDs.size() * sizeof(glm::ivec2), jointIDs.data(), GL_STATIC_DRAW);

        generateVertInfluencecWeights();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufVertInfluenceWeights);      
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, jointWeights.size() * sizeof(glm::vec2), jointWeights.data(), GL_STATIC_DRAW);
    }

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, posVec.size() * sizeof(glm::vec4), posVec.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, norVec.size() * sizeof(glm::vec4), norVec.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, colVec.size() * sizeof(glm::vec4), colVec.data(), GL_STATIC_DRAW);
}

void Mesh::createFromObj(const std::vector<glm::vec3> &verts, const std::vector<std::vector<int>> &faces)
{
    reset();

    std::map<std::pair<int, int>, int> map; // Marks the edge and its existing HE
    std::pair<int,int> p;
    int id1, id2;

    // Create vertices
    for(auto &v : verts)
    {
        m_verts.push_back((mkU<Vertex>(v, nullptr)));
    }

    for(auto &f : faces)
    {
        uPtr<Face> face = mkU<Face>(glm::vec3(glm::gaussRand(0.2f, 0.9f), glm::gaussRand(0.2f, 0.9f), glm::gaussRand(0.f, 1.f)), nullptr);

        // Create HEs
        int n = m_HEs.size();
        for(auto index : f)
        {
            uPtr<HalfEdge> he = mkU<HalfEdge>(nullptr, nullptr, face.get(), nullptr);

            Vertex *vert = m_verts[index].get();
            vert->mp_HE = he.get();
            he->mp_vert = vert;

            m_HEs.push_back(std::move(he));
        }
        int n2 = m_HEs.size();

        // Link and sym up all new HEs
        for(int i = n; i < n2; i++)
        {
            // Link
            m_HEs[i]->mp_next = (i == n2 - 1) ? m_HEs[n].get() : m_HEs[i + 1].get();

            // Sym
            id1 = m_HEs[i]->mp_vert->m_id;
            id2 = (i == n) ? m_HEs[n2 - 1]->mp_vert->m_id : m_HEs[i - 1]->mp_vert->m_id;
            p = id1 < id2 ? std::make_pair(id1, id2) : std::make_pair(id2, id1);
            if(map.find(p) != map.end())
            {
                m_HEs[i]->mp_sym = m_HEs[map[p]].get();
                m_HEs[map[p]]->mp_sym = m_HEs[i].get();
            }
            else
            {
                map[p] = i;
            }
        }

        face->mp_HE = m_HEs[n].get();
        m_faces.push_back(std::move(face));
    }
}

void Mesh::createCube()
{
    reset();

    // Create vertices
    m_verts.clear();
    m_verts.push_back(mkU<Vertex>(glm::vec3(-0.5, -0.5, 0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(0.5, -0.5, 0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(0.5, 0.5, 0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(-0.5, 0.5, 0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(-0.5, -0.5, -0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(0.5, -0.5, -0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(0.5, 0.5, -0.5), nullptr));
    m_verts.push_back(mkU<Vertex>(glm::vec3(-0.5, 0.5, -0.5), nullptr));

    // Create faces. The order is:
    // front(0), right(1), back(2), left(3), up(4), down(5)
    m_faces.clear();
    for(int i = 0; i < 6; i++)
    {
        m_faces.push_back(mkU<Face>(_cubeFaceColor, nullptr));
    }

    // Create half-edges face by face
    m_HEs.clear();
    std::map<std::pair<int, int>, int> map; // Mark edge and its created HE's index
    for(int i = 0; i < 6; i++)
    {
        // Create 4 HEs
        for(int j = 0; j < 4; j++)
        {
            m_HEs.push_back(mkU<HalfEdge>(nullptr, nullptr, m_faces[i].get(), nullptr));
        }

        // Set link between vertex and HE
        for(int j = 0; j < 4; j++)
        {
            auto he = m_HEs[4 * i + j].get();
            auto vert = m_verts[_cubeVertIdxRef[i][(j + 1) % 4]].get();

            he->mp_vert = vert;
            vert->mp_HE = he;
        }

        // Link 4 HEs
        for(int j = 0; j < 4; j++)
        {
            m_HEs[4 * i + j].get()->mp_next = m_HEs[4 * i + (j + 1) % 4].get();
        }

        // Find symmetric HE
        for(int j = 0; j < 4; j++)
        {
            int v0 = _cubeVertIdxRef[i][j];
            int v1 = _cubeVertIdxRef[i][(j + 1) % 4];

            std::pair<int, int> edge = v0 < v1 ? std::make_pair(v0, v1) : std::make_pair(v1, v0);
            if(map.find(edge) != map.end())
            {
                HalfEdge *he = m_HEs[map[edge]].get();
                HalfEdge *he2 = m_HEs[4 * i + j].get();
                he->mp_sym = he2;
                he2->mp_sym = he;
            }
            else
            {
                map[edge] = 4 * i + j;
            }
        }

        // Set face's HE pointer
        m_faces[i].get()->mp_HE = m_HEs[4 * i].get();
    }

    // Create VBO
    create();
}

void Mesh::splitEdge(HalfEdge *he)
{
    HalfEdge *he1 = he;
    HalfEdge *he2 = he->mp_sym;

    // Get the mid point position
    glm::vec3 pos1 = he1->mp_vert->m_pos;
    glm::vec3 pos2;
    glm::vec3 midPos;

    HalfEdge *heBefore = findBeforeHE(he1);
    pos2 = heBefore->mp_vert->m_pos;
    midPos = (pos1 + pos2) * 0.5f;

    // Create 2 new HEs and 1 vertex
    uPtr<HalfEdge> he1b = mkU<HalfEdge>(he1->mp_next, nullptr, he1->mp_face, he1->mp_vert);
    uPtr<HalfEdge> he2b = mkU<HalfEdge>(he2->mp_next, nullptr, he2->mp_face, he2->mp_vert);
    uPtr<Vertex> v = mkU<Vertex>(midPos, he1);

    // Re-link HEs
    he1->mp_next = he1b.get();
    he2->mp_next = he2b.get();

    // Decide sym based on the 2 HEs' directions
    if(he1->mp_vert == he2->mp_vert)
    {
        he1b->mp_sym = he2b.get();
        he2b->mp_sym = he1b.get();
    }
    else
    {
        he1->mp_sym = he2b.get();
        he2b->mp_sym = he1;
        he2->mp_sym = he1b.get();
        he1b->mp_sym = he2;
    }

    // Re-link Vertices
    he1->mp_vert = v.get();
    he2->mp_vert = v.get();

    // Store new HEs and vertex
    m_HEs.push_back(std::move(he1b));
    m_HEs.push_back(std::move(he2b));
    m_verts.push_back(std::move(v));
}

void Mesh::triangulate(Face *face)
{       
    // Count all HEs first
    HalfEdge *st = face->mp_HE;
    HalfEdge *he = st;
    int count = 0;
    do
    {
        count++;
        he = he->mp_next;
    }
    while(he != st);

    // Ear Clipping:
    //  1.Loop with 4 continuous HEs and form triangle (he->mp_vert, he2->mp_vert, he3->mp_vert)
    //  2.Remove formed triangle from the loop
    HalfEdge *he2 = he->mp_next;
    HalfEdge *he3 = he2->mp_next;
    HalfEdge *he4 = he3->mp_next;
    while(count > 3)
    {
        if(checkCanTriangulate(he->mp_vert->m_pos, he2->mp_vert->m_pos, he3->mp_vert->m_pos) &&
           checkCanTriangulate(he->mp_vert->m_pos, he3->mp_vert->m_pos, he4->mp_vert->m_pos))   // Also check(he, he3, he4) cause we don't want
                                                                                                // he4 inside the triangle(he1, he2, he3)
        {
            uPtr<Face> faceAdd = mkU<Face>(_cubeFaceColor, he2);

            uPtr<HalfEdge> he_a = mkU<HalfEdge>(he2, nullptr, faceAdd.get(), he->mp_vert);  // New HE inside the triangle
            uPtr<HalfEdge> he_b = mkU<HalfEdge>(he4, nullptr, face, he3->mp_vert);          // The sym of the new HE
            he_a->mp_sym = he_b.get();
            he_b->mp_sym = he_a.get();

            // Form new triangle face
            he3->mp_next = he_a.get();
            he2->mp_face = faceAdd.get();
            he3->mp_face = faceAdd.get();

            // Connert the rest of polygon
            he->mp_next = he_b.get();
            count--;

            // Store new add
            m_faces.push_back(std::move(faceAdd));
            m_HEs.push_back(std::move(he_a));
            m_HEs.push_back(std::move(he_b));
        }

        // Next loop
        if(he == he4)
        {
            he = he->mp_next;
        }
        else
        {
            he = he4;
        }
        he2 = he->mp_next;
        he3 = he2->mp_next;
        he4 = he3->mp_next;
    }

    // Handle the last 3 HE
    he4->mp_face = face;
    he4->mp_next->mp_face = face;
    he4->mp_next->mp_next->mp_face = face;
    face->mp_HE = he4;
}

void Mesh::subdivision()
{
    createCentriods();
    createSmoothMidPoints();
    smoothOrignalVertices();
    quadranglate();
}

void Mesh::addSkeleton(Skeleton *skeleton)
{
    if(m_verts.size() == 0 || skeleton == nullptr || skeleton->m_joints.size() == 0)
    {
        skeleton = nullptr;
        return;
    }

    // Navie method: Set influence with N closest joints
    uint N = 2;
    for(uint i = 0; i < m_verts.size(); i++)
    {
        Vertex *vert = m_verts[i].get();

        std::vector<uint> joints;
        std::vector<float> distances;

        for(auto &j : skeleton->m_joints)
        {
            Joint *joint = j.get();

            glm::vec3 jointPos = (glm::vec3)(joint->getWorldPosition());
            float distance = glm::distance(jointPos, vert->m_pos);

            if(joints.size() < N)
            {
                joints.push_back(joint->m_id);
                distances.push_back(distance);
                continue;
            }

            for(uint n = 0; n < N; n++)
            {
                if(distance < distances[n])
                {
                    distances[n] = distance;
                    joints[n] = joint->m_id;
                    break;
                }
            }
        }

        float totalDistance = 0;
        for(uint n = 0; n < N; n++)
        {
            totalDistance += distances[n];
        }
        for(uint n = 0; n < N; n++)
        {
            distances[n] = 1 - distances[n] / totalDistance;
        }

        vert->setJointInfluence(joints, distances);
    }

    m_skeleton = skeleton;
}

bool Mesh::checkCanTriangulate(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3)
{
    glm::vec3 v1 = pos1 - pos2;
    glm::vec3 v2 = pos1 - pos3;

    glm::vec3 product = glm::cross(v1, v2);

    return product.x * product.x + product.y * product.y + product.z * product.z > 0;
}

glm::vec4 Mesh::findFaceNormal(Face *face)
{
    HalfEdge *he = face->mp_HE;

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    do
    {
        v0 = he->mp_vert->m_pos;
        v1 = he->mp_next->mp_vert->m_pos;
        v2 = he->mp_next->mp_next->mp_vert->m_pos;

        if(checkCanTriangulate(v0, v1, v2))
        {
            break;
        }

        he = he->mp_next;
    }
    while(true);

    glm::vec4 nor = glm::vec4(glm::normalize(glm::cross(v1 - v0, v2 - v1)), 0);
    return nor;
}

HalfEdge* Mesh::findBeforeHE(HalfEdge *he)
{
    HalfEdge *p = he;
    while(p->mp_next != he)
    {
        p = p->mp_next;
    }

    return p;
}

void Mesh::createCentriods()
{
    m_centroidStartIndex = m_verts.size();

    for(auto &face : m_faces)
    {
        float n = 0;
        glm::vec3 pos = glm::vec3(0, 0, 0);        

        HalfEdge *st = face->mp_HE;
        HalfEdge *he = st;
        do
        {
            pos += he->mp_vert->m_pos;
            n++;
            he = he->mp_next;
        }
        while(he != st);

        m_verts.push_back(mkU<Vertex>(pos / n, nullptr));
    }
}

void Mesh::createSmoothMidPoints()
{
    std::vector<uPtr<HalfEdge>> tempHEs;

    for(auto &he : m_HEs)
    {
        // Skip the HE that aleardy points to a smoothed mid point
        if(he->mp_vert->m_id >= m_centroidStartIndex)
        {
            continue;
        }

        HalfEdge *he2 = he->mp_sym;

        uPtr<HalfEdge> he1b = mkU<HalfEdge>(he->mp_next, nullptr, he->mp_face, he->mp_vert);
        uPtr<HalfEdge> he2b = mkU<HalfEdge>(he2->mp_next, nullptr, he2->mp_face, he2->mp_vert);

        he1b->mp_sym = he2;
        he2->mp_sym = he1b.get();
        he2b->mp_sym = he.get();
        he->mp_sym = he2b.get();

        he->mp_next = he1b.get();
        he2->mp_next = he2b.get();

        float n = 2;
        glm::vec3 pos = he->mp_vert->m_pos + he2->mp_vert->m_pos;

        if(he->mp_face != nullptr)
        {
            pos += m_verts[m_centroidStartIndex + findFaceIndex(he->mp_face)]->m_pos;
            n++;
        }
        if(he2->mp_face != nullptr)
        {
            pos += m_verts[m_centroidStartIndex + findFaceIndex(he2->mp_face)]->m_pos;
            n++;
        }

        uPtr<Vertex> midPoint = mkU<Vertex>(pos / n, he.get());
        he->mp_vert = midPoint.get();
        he2->mp_vert = midPoint.get();

        m_verts.push_back(std::move(midPoint));
        tempHEs.push_back(std::move(he1b));
        tempHEs.push_back(std::move(he2b));
    }

    for(uint i = 0; i < tempHEs.size(); i++)
    {
        m_HEs.push_back(std::move(tempHEs[i]));
    }
}

void Mesh::smoothOrignalVertices()
{
    std::map<uint, std::vector<uint>> map;  // Mark the vertex id and the surrounding HEs that points to a smoothed mid point

    uint vSize = m_centroidStartIndex;
    for(uint i = 0; i < m_HEs.size(); i++)
    {
        HalfEdge *he = m_HEs[i].get();
        uint id = he->mp_next->mp_vert->m_id;
        if(id < vSize)
        {
            map[id].push_back(i);
        }
    }

    for(uint i = 0; i < vSize; i++)
    {
        Vertex *v = m_verts[i].get();

        glm::vec3 midPos(0, 0, 0), centerPos(0, 0, 0);

        for(auto index : map[v->m_id])
        {
            HalfEdge *he = m_HEs[index].get();

            midPos += he->mp_vert->m_pos;
            centerPos += m_verts[m_centroidStartIndex + findFaceIndex(he->mp_face)]->m_pos;
        }

        float n = map[v->m_id].size();
        v->m_pos = ((n - 2) * v->m_pos) / n + midPos / (n * n) + centerPos / (n * n);
    }
}

void Mesh::quadranglate()
{
    std::vector<uPtr<Face>> newFaceVec;

    for(uint i = 0; i < m_faces.size(); i++)
    {
        std::vector<uPtr<HalfEdge>> newHEVec;
        std::vector<HalfEdge*> orignalHELink;

        // Collect all start HEs that points to a smooth midpoint
        HalfEdge *st = m_faces[i]->mp_HE;
        HalfEdge *he = st;
        do
        {
            orignalHELink.push_back(he);
            he = he->mp_next;
        }
        while(he != st);

        // Create and link 2 new HEs for each sub face
        int n = orignalHELink.size();
        for(int j = 0; j < n; j += 2)
        {
            HalfEdge *he1 = orignalHELink[j];
            HalfEdge *heb = orignalHELink[(n + j - 1) % n]; // The HE before he1

            uPtr<HalfEdge> he2 = mkU<HalfEdge>(nullptr, nullptr, nullptr, m_verts[m_centroidStartIndex + i].get());
            uPtr<HalfEdge> he3 = mkU<HalfEdge>(nullptr, nullptr, nullptr, orignalHELink[(n + j - 2) % n]->mp_vert);

            he1->mp_next = he2.get();
            he2->mp_next = he3.get();
            he3->mp_next = heb;

            if(j == 0)
            {
                he2->mp_face = m_faces[i].get();
                he3->mp_face = m_faces[i].get();
            }
            else
            {
                uPtr<Face> newFace = mkU<Face>(m_faces[i]->m_color, he1);
                he1->mp_face = newFace.get();
                he2->mp_face = newFace.get();
                he3->mp_face = newFace.get();
                heb->mp_face = newFace.get();

                newFaceVec.push_back(std::move(newFace));
            }

            newHEVec.push_back(std::move(he2));
            newHEVec.push_back(std::move(he3));
        }

        // Sym up all new HEs and store them
        for(int j = 0; j < n; j += 2)
        {
            HalfEdge *sym1 = orignalHELink[(j + 2) % n]->mp_next->mp_next;
            HalfEdge *sym2 = orignalHELink[(n + j - 2) % n]->mp_next;
            newHEVec[j]->mp_sym = sym1;
            sym1->mp_sym = newHEVec[j].get();
            newHEVec[j + 1]->mp_sym = sym2;
            sym2->mp_sym = newHEVec[j + 1].get();

            m_HEs.push_back(std::move(newHEVec[j]));
            m_HEs.push_back(std::move(newHEVec[j + 1]));
        }
    }

    for(uint i = 0; i < newFaceVec.size(); i++)
    {
        m_faces.push_back(std::move(newFaceVec[i]));
    }
}

uint Mesh::findFaceIndex(Face *face)
{
    for(uint i = 0; i < m_faces.size(); i++)
    {
        if(m_faces[i]->m_id == face->m_id)
            return i;
    }

    return -1;
}
