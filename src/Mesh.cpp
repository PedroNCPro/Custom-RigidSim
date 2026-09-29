#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <exception>
#include <ios>
#include <string>
#include <memory>


Mesh::~Mesh()
{
  clear();
}

void Mesh::computeBoundingSphere(glm::vec3 &center, float &radius) const
{
  center = glm::vec3(0.0);
  radius = 0.f;
  for(const auto &p : _vertexPositions)
    center += p;
  center /= _vertexPositions.size();
  for(const auto &p : _vertexPositions)
    radius = std::max(radius, distance(center, p));
}

void Mesh::recomputePerVertexNormals(bool angleBased)
{
  _vertexNormals.clear();
  // Change the following code to compute a proper per-vertex normal
  _vertexNormals.resize(_vertexPositions.size(), glm::vec3(0.0, 0.0, 0.0));

  for(unsigned int tIt=0 ; tIt < _triangleIndices.size() ; ++tIt) {
    glm::uvec3 t = _triangleIndices[tIt];
    glm::vec3 n_t = glm::cross(
      _vertexPositions[t[1]] - _vertexPositions[t[0]],
      _vertexPositions[t[2]] - _vertexPositions[t[0]]);
    _vertexNormals[t[0]] += n_t;
    _vertexNormals[t[1]] += n_t;
    _vertexNormals[t[2]] += n_t;
  }
  for(unsigned int nIt = 0 ; nIt < _vertexNormals.size() ; ++nIt) {
    glm::normalize(_vertexNormals[nIt]);
  }
}

void Mesh::recomputePerVertexTextureCoordinates()
{
  _vertexTexCoords.clear();
  _vertexTexCoords.resize(_vertexPositions.size(), glm::vec2(0.0, 0.0));

  float xMin = FLT_MAX, xMax = FLT_MIN;
  float yMin = FLT_MAX, yMax = FLT_MIN;
  for(glm::vec3 &p : _vertexPositions) {
    xMin = std::min(xMin, p[0]);
    xMax = std::max(xMax, p[0]);
    yMin = std::min(yMin, p[1]);
    yMax = std::max(yMax, p[1]);
  }
  for(unsigned int pIt = 0 ; pIt < _vertexTexCoords.size() ; ++pIt) {
    _vertexTexCoords[pIt] = glm::vec2(
      (_vertexPositions[pIt][0] - xMin)/(xMax-xMin),
      (_vertexPositions[pIt][1] - yMin)/(yMax-yMin));
  }
}

void Mesh::addPlane(const float square_half_side)
{
  _vertexPositions.push_back(glm::vec3(-square_half_side,-square_half_side, 0));
  _vertexPositions.push_back(glm::vec3(+square_half_side,-square_half_side, 0));
  _vertexPositions.push_back(glm::vec3(+square_half_side,+square_half_side, 0));
  _vertexPositions.push_back(glm::vec3(-square_half_side,+square_half_side, 0));

  _vertexTexCoords.push_back(glm::vec2(0.0, 0.0));
  _vertexTexCoords.push_back(glm::vec2(1.0, 0.0));
  _vertexTexCoords.push_back(glm::vec2(1.0, 1.0));
  _vertexTexCoords.push_back(glm::vec2(0.0, 1.0));

  _vertexNormals.push_back(glm::vec3(0,0, 1));
  _vertexNormals.push_back(glm::vec3(0,0, 1));
  _vertexNormals.push_back(glm::vec3(0,0, 1));
  _vertexNormals.push_back(glm::vec3(0,0, 1));

  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));
}

void Mesh::addSphere(const float radius, const size_t resolution) { // should generate a unit sphere

  int lat_p = resolution;
  int lon_p = resolution;
  std::vector<float> v_pos;
  std::vector<unsigned int> t_ids;
  std::vector<float> v_texcoords;
  std::vector<float> v_normals;

  for (int i = 0; i <= lon_p; ++i) {
    float theta = (float)i / lon_p * 2.0f * M_PI;
    for (int j = 0; j < lat_p; ++j) {
        float phi = (float)j / (lat_p - 1) * M_PI;

        const float x = radius * sin(phi) * sin(theta);
        const float y = radius * cos(phi);
        const float z = radius * sin(phi) * cos(theta);
        v_pos.push_back(x);
        v_pos.push_back(y);
        v_pos.push_back(z);

        float u = (float)i / lon_p;
        float v = (float)j / (lat_p - 1);
        v_texcoords.push_back(u);
        v_texcoords.push_back(v);
    }
  }


  for(int i = 0; i < lon_p; ++i) {
    for(int j = lon_p*i+1; j < lon_p*i+(lat_p-1); ++j) {
      t_ids.push_back(j);
      t_ids.push_back((j+lat_p));
      t_ids.push_back(j-1);

      t_ids.push_back(j);
      t_ids.push_back((j+lat_p+1));
      t_ids.push_back((j+lat_p));
    }
  }

  for(size_t i = 0; i < v_pos.size(); i+=3) {
    glm::vec3 n = glm::normalize(glm::vec3(v_pos[i], v_pos[i+1], v_pos[i+2]));
    v_normals.push_back(n.x);
    v_normals.push_back(n.y);
    v_normals.push_back(n.z);
  }

  _vertexPositions.insert(_vertexPositions.end(), (glm::vec3*)v_pos.data(), (glm::vec3*)(v_pos.data() + v_pos.size()));
  _vertexTexCoords.insert(_vertexTexCoords.end(), (glm::vec2*)v_texcoords.data(), (glm::vec2*)(v_texcoords.data() + v_texcoords.size()));
  _vertexNormals.insert(_vertexNormals.end(), (glm::vec3*)v_normals.data(), (glm::vec3*)(v_normals.data() + v_normals.size()));
  for(size_t i = 0; i < t_ids.size(); i+=3) {
    _triangleIndices.push_back(glm::uvec3(t_ids[i], t_ids[i+1], t_ids[i+2]));
  }

}

void Mesh::addBox(const float w, const float h, const float d, const glm::vec3 &center) {
  // back
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, -0.5*d));
  _vertexTexCoords.push_back(glm::vec2(2.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(2.0, 3.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(0, 0, -1));
  _vertexNormals.push_back(glm::vec3(0, 0, -1));
  _vertexNormals.push_back(glm::vec3(0, 0, -1));
  _vertexNormals.push_back(glm::vec3(0, 0, -1));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));

  // front
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, +0.5*d));
  _vertexTexCoords.push_back(glm::vec2(0.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(1.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(1.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(0.0, 3.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(0, 0, 1));
  _vertexNormals.push_back(glm::vec3(0, 0, 1));
  _vertexNormals.push_back(glm::vec3(0, 0, 1));
  _vertexNormals.push_back(glm::vec3(0, 0, 1));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));
  
  // bottom
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, +0.5*d));
  _vertexTexCoords.push_back(glm::vec2(2.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 4.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(2.0, 4.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(0, -1, 0));
  _vertexNormals.push_back(glm::vec3(0, -1, 0));
  _vertexNormals.push_back(glm::vec3(0, -1, 0));
  _vertexNormals.push_back(glm::vec3(0, -1, 0));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));
  
  // top
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, -0.5*d));
  _vertexTexCoords.push_back(glm::vec2(2.0, 1.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 1.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(2.0, 2.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(0, 1, 0));
  _vertexNormals.push_back(glm::vec3(0, 1, 0));
  _vertexNormals.push_back(glm::vec3(0, 1, 0));
  _vertexNormals.push_back(glm::vec3(0, 1, 0));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));

  // left
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, -0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(-0.5*w, +0.5*h, -0.5*d));
  _vertexTexCoords.push_back(glm::vec2(1.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(2.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(2.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(1.0, 3.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(-1, 0, 0));
  _vertexNormals.push_back(glm::vec3(-1, 0, 0));
  _vertexNormals.push_back(glm::vec3(-1, 0, 0));
  _vertexNormals.push_back(glm::vec3(-1, 0, 0));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));

  // right
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, +0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, -0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, -0.5*d));
  _vertexPositions.push_back(center + glm::vec3(+0.5*w, +0.5*h, +0.5*d));
  _vertexTexCoords.push_back(glm::vec2(3.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(4.0, 2.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(4.0, 3.0)*0.25f);
  _vertexTexCoords.push_back(glm::vec2(3.0, 3.0)*0.25f);
  _vertexNormals.push_back(glm::vec3(1, 0, 0));
  _vertexNormals.push_back(glm::vec3(1, 0, 0));
  _vertexNormals.push_back(glm::vec3(1, 0, 0));
  _vertexNormals.push_back(glm::vec3(1, 0, 0));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-3, _vertexPositions.size()-2));
  _triangleIndices.push_back(
    glm::uvec3(_vertexPositions.size()-4, _vertexPositions.size()-2, _vertexPositions.size()-1));
};

void Mesh::addWeldedBox(const float w, const float h, const float d, const glm::vec3 &center)
{
    const unsigned int base = (unsigned int)_vertexPositions.size();

    // Vertices
    const glm::vec3 p0 = center + glm::vec3(-0.5f*w, -0.5f*h, -0.5f*d);
    const glm::vec3 p1 = center + glm::vec3(+0.5f*w, -0.5f*h, -0.5f*d);
    const glm::vec3 p2 = center + glm::vec3(+0.5f*w, +0.5f*h, -0.5f*d);
    const glm::vec3 p3 = center + glm::vec3(-0.5f*w, +0.5f*h, -0.5f*d);

    const glm::vec3 p4 = center + glm::vec3(-0.5f*w, -0.5f*h, +0.5f*d);
    const glm::vec3 p5 = center + glm::vec3(+0.5f*w, -0.5f*h, +0.5f*d);
    const glm::vec3 p6 = center + glm::vec3(+0.5f*w, +0.5f*h, +0.5f*d);
    const glm::vec3 p7 = center + glm::vec3(-0.5f*w, +0.5f*h, +0.5f*d);

    _vertexPositions.push_back(p0); // 0
    _vertexPositions.push_back(p1); // 1
    _vertexPositions.push_back(p2); // 2
    _vertexPositions.push_back(p3); // 3
    _vertexPositions.push_back(p4); // 4
    _vertexPositions.push_back(p5); // 5
    _vertexPositions.push_back(p6); // 6
    _vertexPositions.push_back(p7); // 7

    // UVs: dummy
    for(int i=0;i<8;i++)
        _vertexTexCoords.push_back(glm::vec2(0.0f));

    // Normals: smooth (works for welded)
    for(int i=0;i<8;i++)
        _vertexNormals.push_back(glm::normalize(_vertexPositions[base+i] - center));

    // Faces (CCW when viewed from outside)

    // -Z (back)
    _triangleIndices.push_back(glm::uvec3(base+0, base+2, base+1));
    _triangleIndices.push_back(glm::uvec3(base+0, base+3, base+2));

    // +Z (front)
    _triangleIndices.push_back(glm::uvec3(base+4, base+5, base+6));
    _triangleIndices.push_back(glm::uvec3(base+4, base+6, base+7));

    // -Y (bottom)
    _triangleIndices.push_back(glm::uvec3(base+0, base+1, base+5));
    _triangleIndices.push_back(glm::uvec3(base+0, base+5, base+4));

    // +Y (top)
    _triangleIndices.push_back(glm::uvec3(base+3, base+6, base+2));
    _triangleIndices.push_back(glm::uvec3(base+3, base+7, base+6));

    // -X (left)
    _triangleIndices.push_back(glm::uvec3(base+0, base+7, base+3));
    _triangleIndices.push_back(glm::uvec3(base+0, base+4, base+7));

    // +X (right)
    _triangleIndices.push_back(glm::uvec3(base+1, base+2, base+6));
    _triangleIndices.push_back(glm::uvec3(base+1, base+6, base+5));
}

void Mesh::meshFromMatrix(const std::vector< std::vector<int> > &shape, float voxel_w, float voxel_h, float voxel_d)
{ 
  glm::vec3 global_center = glm::vec3(0.0);
  int num_boxes = 0;
  std::vector<std::pair<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int> >> rects = find_rects(shape);

  for(const auto &rect : rects) {
    unsigned int i_min = rect.first.first;
    unsigned int i_max = rect.second.first;
    unsigned int j_min = rect.first.second;;
    unsigned int j_max = rect.second.second;
    glm::vec3 local_center = glm::vec3((j_min+j_max+1)*voxel_w/2.0f, (i_min+i_max+1)*(-voxel_h)/2.0f, 0.0);
    global_center += local_center;
    num_boxes++;
    addBox((j_max-j_min+1)*voxel_w, (i_max-i_min+1)*voxel_h, voxel_d, local_center);
  }
  for(auto &p : _vertexPositions) {
    p -= global_center / static_cast<float>(num_boxes);
  }
}

void Mesh::scaleMesh(const float scale)
{
  for(auto &p : _vertexPositions) {
    p *= scale;
  }
}


void Mesh::init()
{
  // Generate a GPU buffer to store the positions of the vertices
  size_t vertexBufferSize = sizeof(glm::vec3)*_vertexPositions.size();
  glGenBuffers(1, &_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, _posVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, _vertexPositions.data(), GL_DYNAMIC_READ);

  // Same for normal
  glGenBuffers(1, &_normalVbo);
  glBindBuffer(GL_ARRAY_BUFFER, _normalVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, _vertexNormals.data(), GL_DYNAMIC_READ);

  // Same for texture coordinates
  size_t texCoordBufferSize = sizeof(glm::vec2)*_vertexTexCoords.size();
  glGenBuffers(1, &_texCoordVbo);
  glBindBuffer(GL_ARRAY_BUFFER, _texCoordVbo);
  glBufferData(GL_ARRAY_BUFFER, texCoordBufferSize, _vertexTexCoords.data(), GL_DYNAMIC_READ);

  // Same for the index buffer that stores the list of indices of the triangles forming the mesh
  size_t indexBufferSize = sizeof(glm::uvec3)*_triangleIndices.size();
  glGenBuffers(1, &_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, _triangleIndices.data(), GL_DYNAMIC_READ);

  // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, _posVbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, _normalVbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, _texCoordVbo);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

  glBindVertexArray(0); // Desactive the VAO just created. Will be activated at rendering time.
}

void Mesh::render()
{
  glBindVertexArray(_vao);      // Activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_triangleIndices.size()*3), GL_UNSIGNED_INT, 0);
  // Call for rendering: stream the current GPU geometry through the current GPU program
}

void Mesh::clear()
{
  _vertexPositions.clear();
  _vertexNormals.clear();
  _vertexTexCoords.clear();
  _triangleIndices.clear();
  if(_vao) {
    glDeleteVertexArrays(1, &_vao);
    _vao = 0;
  }
  if(_posVbo) {
    glDeleteBuffers(1, &_posVbo);
    _posVbo = 0;
  }
  if(_normalVbo) {
    glDeleteBuffers(1, &_normalVbo);
    _normalVbo = 0;
  }
  if(_texCoordVbo) {
    glDeleteBuffers(1, &_texCoordVbo);
    _texCoordVbo = 0;
  }
  if(_ibo) {
    glDeleteBuffers(1, &_ibo);
    _ibo = 0;
  }
}


void loadOFF(const std::string &filename, std::shared_ptr<Mesh> meshPtr)
{
  std::cout << " > Start loading mesh <" << filename << ">" << std::endl;
  meshPtr->clear();
  std::ifstream in(filename.c_str());
  if(!in)
    throw std::ios_base::failure("[Mesh Loader][loadOFF] Cannot open " + filename);
  std::string offString;
  unsigned int sizeV, sizeT, tmp;
  in >> offString >> sizeV >> sizeT >> tmp;
  auto &P = meshPtr->vertexPositions();
  auto &T = meshPtr->triangleIndices();
  P.resize(sizeV);
  T.resize(sizeT);
  size_t tracker = (sizeV + sizeT)/20;
  std::cout << " > [" << std::flush;
  for(unsigned int i=0; i<sizeV; ++i) {
    if(i % tracker == 0)
      std::cout << "-" << std::flush;
    in >> P[i][0] >> P[i][1] >> P[i][2];
  }
  int s;
  for(unsigned int i=0; i<sizeT; ++i) {
    if((sizeV + i) % tracker == 0)
      std::cout << "-" << std::flush;
    in >> s;
    for(unsigned int j=0; j<3; ++j)
      in >> T[i][j];
  }
  std::cout << "]" << std::endl;
  in.close();
  meshPtr->vertexNormals().resize(P.size(), glm::vec3(0.f, 0.f, 1.f));
  meshPtr->vertexTexCoords().resize(P.size(), glm::vec2(0.f, 0.f));
  meshPtr->recomputePerVertexNormals();
  meshPtr->recomputePerVertexTextureCoordinates();
  std::cout << " > Mesh <" << filename << "> loaded" <<  std::endl;
}

std::vector<std::pair<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>> find_rects(std::vector<std::vector<int>> shape)
{
  std::vector<std::pair<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>> res;
  int num_rows = shape.size();
  int num_cols = shape[0].size();
  for(int i=0; i<num_rows; ++i) {
    for(int j=0; j<num_cols; ++j) {
      if(shape[i][j]) {
        int first_i = i, first_j = j;
        int last_i = i, last_j = j;
        int last_i_b = last_i, last_j_b = last_j;
        shape[i][j] = 0;
        bool rect_found = false;
        std::vector<std::vector<int>> shape_copy = shape;
        while(!rect_found) {
          if(last_j+1 < num_cols){
            int k;
            for(k=first_i; k<=last_i; ++k) {
              if(shape[k][last_j+1]) {
                shape[k][last_j+1] = 0;
              }
              else {
                shape = shape_copy;
                k--;
                break;
              }
            }
            if(k-1 == last_i) {
              last_j++;
              shape_copy = shape;
            }
          }
          if(last_i+1 < num_rows) {
            int k;
            for(k=first_j; k<=last_j; ++k) {
              if(shape[last_i+1][k]) {
                shape[last_i+1][k] = 0;
              }
              else {
                shape = shape_copy;
                k--;
                break;
              }
            }
            if(k-1 == last_j) {
              last_i++;
              shape_copy = shape;
            }
          }
          if(last_i == last_i_b && last_j == last_j_b) {
            rect_found = true;
          }
          else {
            last_i_b = last_i;
            last_j_b = last_j;
          }
        }
        res.push_back({{first_i, first_j}, {last_i_b, last_j_b}});
      }
    }
  }
  return res;
}