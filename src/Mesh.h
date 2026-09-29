#ifndef MESH_H
#define MESH_H

#include <glad/gl.h>
#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <map>
#include <set>

class Mesh {
public:
  virtual ~Mesh();

  const std::vector<glm::vec3> &vertexPositions() const { return _vertexPositions; }
  std::vector<glm::vec3> &vertexPositions() { return _vertexPositions; }

  const std::vector<glm::vec3> &vertexNormals() const { return _vertexNormals; }
  std::vector<glm::vec3> &vertexNormals() { return _vertexNormals; }

  const std::vector<glm::vec2> &vertexTexCoords() const { return _vertexTexCoords; }
  std::vector<glm::vec2> &vertexTexCoords() { return _vertexTexCoords; }

  const std::vector<glm::uvec3> &triangleIndices() const { return _triangleIndices; }
  std::vector<glm::uvec3> &triangleIndices() { return _triangleIndices; }

  // Compute the parameters of a sphere which bounds the mesh
  void computeBoundingSphere(glm::vec3 &center, float &radius) const;

  void recomputePerVertexNormals(bool angleBased = false);
  void recomputePerVertexTextureCoordinates( );

  void init();
  void render();
  void clear();

  void addPlane(float square_half_side = 1.0f);
  void addBox(const float w, const float h, const float d, const glm::vec3 &center = glm::vec3(0,0,0));
  void addSphere(const float radius, const size_t resolution=16);
  void addWeldedBox(const float w, const float h, const float d, const glm::vec3 &center);
  void meshFromMatrix(const std::vector< std::vector<int> > &shape, float voxel_w = 0.1f, float voxel_h = 0.1f, float voxel_d = 0.1f);

  void scaleMesh(const float scale);

  void subdivideLinear()
  {
    std::vector<glm::vec3> newVertices = _vertexPositions;
    std::vector<glm::uvec3> newTriangles;

    struct Edge {
      unsigned int a , b;
      Edge( unsigned int c , unsigned int d ) : a( std::min<unsigned int>(c,d) ) , b( std::max<unsigned int>(c,d) ) {}
      bool operator < ( Edge const & o ) const {   return a < o.a  ||  (a == o.a && b < o.b);  }
      bool operator == ( Edge const & o ) const {   return a == o.a  &&  b == o.b;  }
    };
    std::map< Edge , unsigned int > newVertexOnEdge;
    for(unsigned int tIt = 0 ; tIt < _triangleIndices.size() ; ++tIt) {
      unsigned int a = _triangleIndices[tIt][0];
      unsigned int b = _triangleIndices[tIt][1];
      unsigned int c = _triangleIndices[tIt][2];


      Edge Eab(a,b);
      unsigned int oddVertexOnEdgeEab = 0;
      if( newVertexOnEdge.find( Eab ) == newVertexOnEdge.end() ) {
        newVertices.push_back( (_vertexPositions[ a ] + _vertexPositions[ b ]) / 2.f );
        oddVertexOnEdgeEab = newVertices.size() - 1;
        newVertexOnEdge[Eab] = oddVertexOnEdgeEab;
      }
      else { oddVertexOnEdgeEab = newVertexOnEdge[Eab]; }


      Edge Ebc(b,c);
      unsigned int oddVertexOnEdgeEbc = 0;
      if( newVertexOnEdge.find( Ebc ) == newVertexOnEdge.end() ) {
        newVertices.push_back( (_vertexPositions[ b ] + _vertexPositions[ c ]) / 2.f );
        oddVertexOnEdgeEbc = newVertices.size() - 1;
        newVertexOnEdge[Ebc] = oddVertexOnEdgeEbc;
      }
      else { oddVertexOnEdgeEbc = newVertexOnEdge[Ebc]; }

      Edge Eca(c,a);
      unsigned int oddVertexOnEdgeEca = 0;
      if( newVertexOnEdge.find( Eca ) == newVertexOnEdge.end() ) {
        newVertices.push_back( (_vertexPositions[ c ] + _vertexPositions[ a ]) / 2.f );
        oddVertexOnEdgeEca = newVertices.size() - 1;
        newVertexOnEdge[Eca] = oddVertexOnEdgeEca;
      }
      else { oddVertexOnEdgeEca = newVertexOnEdge[Eca]; }

      // set new triangles :
      newTriangles.push_back( glm::uvec3( a , oddVertexOnEdgeEab , oddVertexOnEdgeEca ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEab , b , oddVertexOnEdgeEbc ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEca , oddVertexOnEdgeEbc , c ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEab , oddVertexOnEdgeEbc , oddVertexOnEdgeEca ) );
    }

    // after that:
    _triangleIndices = newTriangles;
    _vertexPositions = newVertices;
    recomputePerVertexNormals( );
    recomputePerVertexTextureCoordinates( );
  }

  void subdivideLoopNew()
  {
    // Declare new vertices and new triangles. Initialize the new positions for the even vertices with (0,0,0):
    std::vector<glm::vec3> newVertices( _vertexPositions.size() , glm::vec3(0,0,0) );
    std::vector<glm::uvec3> newTriangles;

    struct Edge {
      unsigned int a , b;
      Edge( unsigned int c , unsigned int d ) : a( std::min<unsigned int>(c,d) ) , b( std::max<unsigned int>(c,d) ) {}
      bool operator < ( Edge const & o ) const {   return a < o.a  ||  (a == o.a && b < o.b);  }
      bool operator == ( Edge const & o ) const {   return a == o.a  &&  b == o.b;  }
    };

    std::map< Edge , unsigned int > newVertexOnEdge; 
    std::map< Edge , std::set< unsigned int > > trianglesOnEdge; 
    std::vector< std::set< unsigned int > > neighboringVertices( _vertexPositions.size() );
    std::vector< bool > evenVertexIsBoundary( _vertexPositions.size() , false );


    // I) First, compute the valences of the even vertices, the neighboring vertices required to update the position of the even vertices, and the boundaries:
    for(unsigned int tIt = 0 ; tIt < _triangleIndices.size() ; ++tIt) {
      unsigned int a = _triangleIndices[tIt][0];
      unsigned int b = _triangleIndices[tIt][1];
      unsigned int c = _triangleIndices[tIt][2];


      neighboringVertices[ a ].insert( b );
      neighboringVertices[ a ].insert( c );
      neighboringVertices[ b ].insert( a );
      neighboringVertices[ b ].insert( c );
      neighboringVertices[ c ].insert( a );
      neighboringVertices[ c ].insert( b );

      //This triangle is on all 3 edges
      trianglesOnEdge[Edge(a,b)].insert(tIt);
      trianglesOnEdge[Edge(b,c)].insert(tIt);
      trianglesOnEdge[Edge(c,a)].insert(tIt);

    }

    // The valence of a vertex is the number of adjacent vertices:
    std::vector< unsigned int > evenVertexValence( _vertexPositions.size() , 0 );
    for( unsigned int v = 0 ; v < _vertexPositions.size() ; ++v ) {
      evenVertexValence[ v ] = neighboringVertices[ v ].size();
    }

    // II) Then, compute the positions for the even vertices: (make sure that you handle the boundaries correctly)
    for(unsigned int v = 0 ; v < _vertexPositions.size() ; ++v) {

      // Identify boundary edges and mark even vertices on boundary
      for(auto nbr : neighboringVertices[v]) {
        if(trianglesOnEdge[Edge(v, nbr)].size() == 1) {
          evenVertexIsBoundary[v] = true;
        }
      }


      const glm::vec3 &P = _vertexPositions[v];

      if(evenVertexIsBoundary[v]) {
        // Boundary vertex
        // Find the two boundary neighbors (those connected by a boundary edge)
        std::vector<unsigned int> boundaryNbrs;
        for(auto nbr : neighboringVertices[v]) {
          if(trianglesOnEdge[Edge(v, nbr)].size() == 1) {
            boundaryNbrs.push_back(nbr);
          }
        }
        if(boundaryNbrs.size() >= 2) {
          // Take the first two boundary neighbors
          glm::vec3 Bsum = _vertexPositions[boundaryNbrs[0]] + _vertexPositions[boundaryNbrs[1]];
          newVertices[v] = 3.0f/4.0f * P + 1.0f/8.0f * Bsum;
        }
        else {
          // Fallback: keep position
          newVertices[v] = P;
        }
      }
      else {
        // Interior vertex
        unsigned int n = evenVertexValence[v];
        if(n == 0) { newVertices[v] = P; continue; }
        // Compute beta using the standard Loop formula
        float u = 3.0f/8.0f + 1.0f/4.0f * cos(2.0f * M_PI / static_cast<float>(n));
        float beta = (5.0f/8.0f - u*u) * 1.0f / static_cast<float>(n);
        glm::vec3 S(0.0f);
        for(auto nbr : neighboringVertices[v]) S += _vertexPositions[nbr];
        newVertices[v] = (1.0f - static_cast<float>(n) * beta) * P + beta * S;
      }
    }

    auto updateOddVertex = [&](unsigned int oddVertexOnEdge, unsigned int v1, unsigned int v2){

      Edge E(v1, v2); 
      if(newVertices[oddVertexOnEdge] == glm::vec3(0,0,0)) {
          if(trianglesOnEdge[E].size() == 1) {
            // Boundary edge
            // Simple midpoint
            newVertices[oddVertexOnEdge] = (_vertexPositions[v1] + _vertexPositions[v2]) / 2.0f;
          }
          else {
            // Interior edge
            // Find the two opposite vertices across the two triangles
            std::vector<unsigned int> opp;
            for(auto tIdx : trianglesOnEdge[E]) {
              for(int k=0;k<3;++k) {
                unsigned int vv = _triangleIndices[tIdx][k];
                if(vv != v1 && vv != v2) opp.push_back(vv);
              }
            }
            if(opp.size() >= 2) {
              newVertices[oddVertexOnEdge] = 3.0f/8.0f * (_vertexPositions[v1] + _vertexPositions[v2]) + 1.0f/8.0f * (_vertexPositions[opp[0]] + _vertexPositions[opp[1]]);
            } else {
              // Fallback
              newVertices[oddVertexOnEdge] = (_vertexPositions[v1] + _vertexPositions[v2]) / 2.0f;
            }
          }
        }
    };


    // III) Then, compute the odd vertices:
    for(unsigned int tIt = 0 ; tIt < _triangleIndices.size() ; ++tIt) {
      unsigned int a = _triangleIndices[tIt][0];
      unsigned int b = _triangleIndices[tIt][1];
      unsigned int c = _triangleIndices[tIt][2];
      

      Edge Eab(a,b);
      unsigned int oddVertexOnEdgeEab = 0;
      if( newVertexOnEdge.find( Eab ) == newVertexOnEdge.end() ) {
        newVertices.push_back( glm::vec3(0,0,0) );
        oddVertexOnEdgeEab = newVertices.size() - 1;
        newVertexOnEdge[Eab] = oddVertexOnEdgeEab;
      }
      else { oddVertexOnEdgeEab = newVertexOnEdge[Eab]; }

      // Update odd vertices
      updateOddVertex(oddVertexOnEdgeEab, a, b);


      Edge Ebc(b,c);
      unsigned int oddVertexOnEdgeEbc = 0;
      if( newVertexOnEdge.find( Ebc ) == newVertexOnEdge.end() ) {
        newVertices.push_back( glm::vec3(0,0,0) );
        oddVertexOnEdgeEbc = newVertices.size() - 1;
        newVertexOnEdge[Ebc] = oddVertexOnEdgeEbc;
      }
      else { oddVertexOnEdgeEbc = newVertexOnEdge[Ebc]; }

      updateOddVertex(oddVertexOnEdgeEbc, b, c);


      Edge Eca(c,a);
      unsigned int oddVertexOnEdgeEca = 0;
      if( newVertexOnEdge.find( Eca ) == newVertexOnEdge.end() ) {
        newVertices.push_back( glm::vec3(0,0,0) );
        oddVertexOnEdgeEca = newVertices.size() - 1;
        newVertexOnEdge[Eca] = oddVertexOnEdgeEca;
      }
      else { oddVertexOnEdgeEca = newVertexOnEdge[Eca]; }

      updateOddVertex(oddVertexOnEdgeEca, c, a);


      // set new triangles :
      newTriangles.push_back( glm::uvec3( a , oddVertexOnEdgeEab , oddVertexOnEdgeEca ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEab , b , oddVertexOnEdgeEbc ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEca , oddVertexOnEdgeEbc , c ) );
      newTriangles.push_back( glm::uvec3( oddVertexOnEdgeEab , oddVertexOnEdgeEbc , oddVertexOnEdgeEca ) );
    }


    _triangleIndices = newTriangles;
    _vertexPositions = newVertices;
    recomputePerVertexNormals( );
    recomputePerVertexTextureCoordinates( );
  }

  void subdivideLoop()
  {
    // subdivideLinear();
    subdivideLoopNew();
  }

private:
  std::vector<glm::vec3> _vertexPositions;
  std::vector<glm::vec3> _vertexNormals;
  std::vector<glm::vec2> _vertexTexCoords;
  std::vector<glm::uvec3> _triangleIndices;

  GLuint _vao = 0;
  GLuint _posVbo = 0;
  GLuint _normalVbo = 0;
  GLuint _texCoordVbo = 0;
  GLuint _ibo = 0;
};

// utility: loader
void loadOFF(const std::string &filename, std::shared_ptr<Mesh> meshPtr);
std::vector<std::pair<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>> find_rects(std::vector<std::vector<int>> shape);

#endif  // MESH_H
