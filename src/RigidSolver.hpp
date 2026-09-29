// ----------------------------------------------------------------------------
// RigidSolver.hpp
//
//  Created on: 18 Dec 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: Simple Rigid Body Solver (DO NOT DISTRIBUTE!)
//
// Copyright 2020-2026 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#ifndef _RIGIDSOLVER_HPP_
#define _RIGIDSOLVER_HPP_

#include <glm/ext/matrix_transform.hpp>

#include "Vector3.hpp"
#include "Matrix3x3.hpp"
#include "Quaternion.hpp"

struct BodyAttributes {
  BodyAttributes() :
    X(0.0, 0, 0), R(Mat3f::I()), P(0, 0, 0), L(0, 0, 0),
    V(0, 0, 0), omega(0, 0, 0), F(0, 0, 0), tau(0, 0, 0)
    {X0 = X; R0 = R; V0 = V; omega0 = omega;}

  glm::mat4 worldMat() const
  {
    return glm::mat4(           // column-major
      R(0,0), R(1,0), R(2,0), 0,
      R(0,1), R(1,1), R(2,1), 0,
      R(0,2), R(1,2), R(2,2), 0,
      X[0],   X[1],   X[2],   1);
  }

  void reset() {
    X = X0; R = R0; V = V0; omega = omega0;
    Iinv = R * I0inv * R.transposed();
    P = M * V;
    L = R * I0 * R.transposed() * omega;
  }

  tReal M;                      // mass
  Mat3f I0, I0inv;              // inertia tensor and its inverse in body space
  Mat3f Iinv;                   // inverse of inertia tensor

  // rigid body state
  Vec3f X;                      // position
  Mat3f R;                      // rotation
  Vec3f P;                      // linear momentum
  Vec3f L;                      // angular momentum

  // auxiliary quantities
  Vec3f V;                      // linear velocity
  Vec3f omega;                  // angular velocity

  // initial states
  Vec3f X0;                     // initial position
  Mat3f R0;                     // initial rotation
  Vec3f V0;                     // initial linear velocity
  Vec3f omega0;                 // initial angular velocity

  // force and torque
  Vec3f F;                      // force
  Vec3f tau;                    // torque

  // mesh's vertices in body space
  // std::vector<Vec3f> vdata0;
  std::vector<glm::vec3> glm_vdata0;
  std::vector<std::pair<int, int>> edge_ids;

};

class Box : public BodyAttributes {
public:
  explicit Box(
    const tReal w=1.0, const tReal h=1.0, const tReal d=1.0, const tReal dens=10.0,
    const Vec3f v0=Vec3f(0, 0, 0), const Vec3f omega_init=Vec3f(0, 0, 0.0)) :
    width(w), height(h), depth(d)
  {
    V0 = v0;                               // initial velocity
    omega0 = omega_init;                       // initial angular velocity

    M = dens * w * h * d;                 // mass

    I0 = Mat3f(                           // initial inertia tensor in body space
      (1.0/12.0)*M*(h*h + d*d), 0, 0,
      0, (1.0/12.0)*M*(w*w + d*d), 0,
      0, 0, (1.0/12.0)*M*(w*w + h*h));

    I0inv = Mat3f(                        // inverse of initial inertia tensor in body space
      (12.0/M)/(h*h + d*d), 0, 0,
      0, (12.0/M)/(w*w + d*d), 0,
      0, 0, (12.0/M)/(w*w + h*h));


    reset();

    // vertices data (8 vertices)
    // vdata0.push_back(Vec3f(-0.5*w, -0.5*h, -0.5*d));
    // vdata0.push_back(Vec3f( 0.5*w, -0.5*h, -0.5*d));
    // vdata0.push_back(Vec3f( 0.5*w,  0.5*h, -0.5*d));
    // vdata0.push_back(Vec3f(-0.5*w,  0.5*h, -0.5*d));

    // vdata0.push_back(Vec3f(-0.5*w, -0.5*h,  0.5*d));
    // vdata0.push_back(Vec3f( 0.5*w, -0.5*h,  0.5*d));
    // vdata0.push_back(Vec3f( 0.5*w,  0.5*h,  0.5*d));
    // vdata0.push_back(Vec3f(-0.5*w,  0.5*h,  0.5*d));

    // also store as glm::vec3 to avoid many conversions in collision detection
    glm_vdata0.push_back(glm::vec3(-0.5f*w, -0.5f*h, -0.5f*d));
    glm_vdata0.push_back(glm::vec3( 0.5f*w, -0.5f*h, -0.5f*d));
    glm_vdata0.push_back(glm::vec3( 0.5f*w,  0.5f*h, -0.5f*d));
    glm_vdata0.push_back(glm::vec3(-0.5f*w,  0.5f*h, -0.5f*d));
    
    glm_vdata0.push_back(glm::vec3(-0.5f*w, -0.5f*h,  0.5f*d));
    glm_vdata0.push_back(glm::vec3( 0.5f*w, -0.5f*h,  0.5f*d));
    glm_vdata0.push_back(glm::vec3( 0.5f*w,  0.5f*h,  0.5f*d));
    glm_vdata0.push_back(glm::vec3(-0.5f*w,  0.5f*h,  0.5f*d));

    edge_ids.push_back({0,1});
    edge_ids.push_back({1,2});
    edge_ids.push_back({2,3});
    edge_ids.push_back({3,0});
    edge_ids.push_back({4,5});
    edge_ids.push_back({5,6});
    edge_ids.push_back({6,7});
    edge_ids.push_back({7,4});
    edge_ids.push_back({0,4});
    edge_ids.push_back({1,5});
    edge_ids.push_back({2,6});
    edge_ids.push_back({3,7});

  }

  // rigid body property
  tReal width, height, depth;
};

class BoxedBody : public BodyAttributes {
public:
  explicit BoxedBody(
    const std::shared_ptr<Mesh> &mesh,
    const tReal w=1.0, const tReal h=1.0, const tReal d=1.0, const tReal dens=10.0,
    const Vec3f v0=Vec3f(0, 0, 0), const Vec3f omega_init=Vec3f(0, 0, 0.0)) :
    _mesh(mesh), width(w), height(h), depth(d)
  {
    V0 = v0;                               // initial velocity
    omega0 = omega_init;                       // initial angular velocity

    M = dens * w * h * d;                 // mass

    I0 = Mat3f(                           // initial inertia tensor in body space
      (1.0/12.0)*M*(h*h + d*d), 0, 0,
      0, (1.0/12.0)*M*(w*w + d*d), 0,
      0, 0, (1.0/12.0)*M*(w*w + h*h));

    I0inv = Mat3f(                        // inverse of initial inertia tensor in body space
      (12.0/M)/(h*h + d*d), 0, 0,
      0, (12.0/M)/(w*w + d*d), 0,
      0, 0, (12.0/M)/(w*w + h*h));

    reset();

    updateGeometry();

  }

  void updateGeometry() {
    glm_vdata0.clear();
    edge_ids.clear();
    for(const auto &v : _mesh->vertexPositions()) {
      // vdata0.push_back(Vec3f(v.x, v.y, v.z));
      glm_vdata0.push_back(v);
    }

    for(const auto &tri : _mesh->triangleIndices()) {
      edge_ids.push_back({tri.x, tri.y});
      edge_ids.push_back({tri.y, tri.z});
      edge_ids.push_back({tri.z, tri.x});
    }
  }

  // rigid body property
  tReal width, height, depth;
  std::shared_ptr<Mesh> _mesh;
};

class RoundBody : public BodyAttributes {
public:
  explicit RoundBody(
    const std::shared_ptr<Mesh> &mesh,
    const tReal r=1.0, const tReal dens=10.0,
    const Vec3f v0=Vec3f(0, 0, 0), const Vec3f omega_init=Vec3f(0, 0, 0.0)) :
    _mesh(mesh), radius(r)
  {
    V0 = v0;                               // initial velocity
    omega0 = omega_init;                       // initial angular velocity

    M = dens * 4.0/3.0 * M_PI * r*r*r;                 // mass

    I0 = Mat3f(                           // initial inertia tensor in body space
      (2.0/5.0)*M*(r*r), 0, 0,
      0, (2.0/5.0)*M*(r*r), 0,
      0, 0, (2.0/5.0)*M*(r*r));

    I0inv = Mat3f(                        // inverse of initial inertia tensor in body space
      (5.0/2.0)/(M*r*r), 0, 0,
      0, (5.0/2.0)/(M*r*r), 0,
      0, 0, (5.0/2.0)/(M*r*r));

    reset();

    updateGeometry();

  }

  void updateGeometry() {
    glm_vdata0.clear();
    edge_ids.clear();
    for(const auto &v : _mesh->vertexPositions()) {
      // vdata0.push_back(Vec3f(v.x, v.y, v.z));
      glm_vdata0.push_back(v);
    }

    for(const auto &tri : _mesh->triangleIndices()) {
      edge_ids.push_back({tri.x, tri.y});
      edge_ids.push_back({tri.y, tri.z});
      edge_ids.push_back({tri.z, tri.x});
    }
  }

  // rigid body property
  tReal radius;
  std::shared_ptr<Mesh> _mesh;
};



class RigidSolver {
public:
  explicit RigidSolver(
    BodyAttributes *body0=nullptr, const Vec3f g=Vec3f(0, 0, 0)) :
    body(body0), _g(g), _step(0), _sim_t(0) {}

  void init()
  {
    _step = 0;
    _sim_t = 0;
  }

  void applyForceAndTorque(tReal dt)
  {
    computeForceAndTorque();
    body->P += body->F * dt;
    body->L += body->tau * dt;
  }

  void step(const tReal dt)
  { 
    body->V = body->P / body->M;
    body->Iinv = body->R * body->I0inv * body->R.transposed();
    body->omega = body->Iinv * body->L;

    body->X += body->V * dt;
    updateRotationMatrix(dt);

    body->F   = Vec3f(0,0,0);
    body->tau = Vec3f(0,0,0);
    
  }

  void solveCollision(const Mesh &body2Mesh, const glm::mat4 &body2WorldMat, tReal restitution=1.0f, tReal damping=0.00f)
  {
    glm::vec3 finalNormal(0.0f, 0.0f, 0.0f);
    glm::vec3 contactPoint(0.0f, 0.0f, 0.0f);
    int contactCount = 0;
    for(auto triangle : body2Mesh.triangleIndices()) {
      glm::vec3 v0 = body2WorldMat * glm::vec4(body2Mesh.vertexPositions()[triangle.x], 1.0f);
      glm::vec3 v1 = body2WorldMat * glm::vec4(body2Mesh.vertexPositions()[triangle.y], 1.0f);
      glm::vec3 v2 = body2WorldMat * glm::vec4(body2Mesh.vertexPositions()[triangle.z], 1.0f);

      std::vector<glm::vec3> tri_vert = {v0, v1, v2};
      glm::vec3 normal = glm::normalize(glm::cross(v1-v0, v2-v0));

      std::vector<glm::vec3> colVerts;

      // VERTEX - FACE COLLISION
      for(glm::vec3 bodyVert : body->glm_vdata0) {
        glm::vec4 worldVert4 = body->worldMat() * glm::vec4(bodyVert, 1.0f);
        glm::vec3 worldVert(worldVert4.x, worldVert4.y, worldVert4.z);
        bool tested = false;
        for(glm::vec3 colVert : colVerts){
          if(colVert == worldVert) tested = true;
        }
        if(!tested){
          glm::vec3 toBody = glm::vec3(body->X.x, body->X.y, body->X.z) - worldVert;
          if(glm::dot(normal, toBody) < 0) normal = -normal; // make sure the normal is pointing outward

          float dist = glm::dot(normal, worldVert - v0);
          glm::vec3 projPoint = worldVert - dist * normal;
          if(abs(dist) < 0.01f && is_in_triangle(projPoint, v0, v1, v2)) { // collision detected
            finalNormal = (finalNormal + normal);
            contactPoint += worldVert;
            colVerts.push_back(worldVert);
            contactCount++;
          }
        }
      }

      // EDGE - EDGE COLLISION
      for(const auto &edge_id : body->edge_ids) {
        glm::vec3 p0 = body->worldMat() * glm::vec4(body->glm_vdata0[edge_id.first], 1.0f);
        glm::vec3 p1 = body->worldMat() * glm::vec4(body->glm_vdata0[edge_id.second], 1.0f);
        bool tested1 = false;
        bool tested2 = false;
        for(glm::vec3 colVert : colVerts){
          if(colVert == p0) tested1 = true;
          if(colVert == p1) tested2 = true;
        }
        if(!tested1 || !tested2){
          glm::vec3 body_edge = p1 - p0;
          for(int i=0; i<3; ++i) {
            glm::vec3 mesh_edge = tri_vert[(i+1)%3] - tri_vert[i];
            glm::vec3 edgeNormal = glm::normalize(glm::cross(body_edge, mesh_edge));
            float dist = glm::dot(edgeNormal, p0 - tri_vert[i]);
              
            glm::vec3 planeNormal = glm::normalize(glm::cross(body_edge, edgeNormal));
            float t = glm::dot(planeNormal, p0 - tri_vert[i]) / glm::dot(planeNormal, mesh_edge);
            glm::vec3 body_inter_v = (t * mesh_edge) + tri_vert[i] + (dist * edgeNormal) - p0;
            float t2 = (body_inter_v.x/body_edge.x + body_inter_v.y/body_edge.y + body_inter_v.z/body_edge.z)/3.0f;
            if(abs(dist) < 0.005f && 0.0f < t && t < 1.0f && 0.0f < t2 && t2 < 1.0f){
              glm::vec3 toBody = glm::vec3(body->X.x, body->X.y, body->X.z) - (body_inter_v + p0);
              if(glm::dot(edgeNormal, toBody) < 0) edgeNormal = -edgeNormal; // make sure the normal is pointing outward              
              finalNormal = (finalNormal + edgeNormal);
              contactPoint += body_inter_v + p0;
              contactCount++;
            }
          }
        }
      }
    }
    // MOMENTUM UPDATE
    if (contactCount > 0){
      contactPoint /= (float)contactCount;

      Vec3f n = Vec3f(finalNormal.x, finalNormal.y, finalNormal.z).normalize();
      Vec3f r = Vec3f(contactPoint.x, contactPoint.y, contactPoint.z) - body->X;
      Vec3f v_contact = body->P / body->M + body->omega.crossProduct(r);
      tReal vn = v_contact.dotProduct(n);
      if (vn < 0.0f){
          tReal denom = (1.0 / body->M) + n.dotProduct(body->Iinv * r.crossProduct(n).crossProduct(r));
          Vec3f impulse = (-(1.0 + restitution) * vn / denom) * n;

          body->P += impulse;
          body->L += r.crossProduct(impulse);
          body->P *= (1.0f - damping);
          body->L *= (1.0f - damping);
      }
    }


  }

  void updateSolver(tReal dt){++_step; _sim_t += dt;}

  BodyAttributes *body;

private:
  void computeForceAndTorque()
  {
    body->F += body->M * _g;
    
    if(_step==1) {
      Vec3f extra_F = Vec3f(0.15, 0.25, 0.03);
      body->F += extra_F;
      Vec3f r = Vec3f(body->glm_vdata0[0].x, body->glm_vdata0[0].y, body->glm_vdata0[0].z);
      body->tau += r.crossProduct(extra_F);
    }

  }

  void updateRotationMatrix(const tReal dt)
  {   
    Quatf q(body->R);
    q += Quatf(0, body->omega.x, body->omega.y, body->omega.z).quatProduct(q) * (0.5f) * dt;
    q.normalize();
    body->R = q.toRotationMatrix();
  }

  bool is_in_triangle(const glm::vec3 &pt, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
  {
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v1v2 = v2 - v1;
    glm::vec3 v2v0 = v0 - v2;
    glm::vec3 n = glm::cross(v0v1, v1v2);
    float eps = 1e-6;
    if(glm::dot(glm::cross(v0v1, pt - v0), n) < -eps) return false;
    if(glm::dot(glm::cross(v1v2, pt - v1), n) < -eps) return false;
    if(glm::dot(glm::cross(v2v0, pt - v2), n) < -eps) return false;
    return true;
  }


  // simulation parameters
  Vec3f _g;                     // gravity
  tIndex _step;                 // step count
  tReal _sim_t;                 // simulation time
};

#endif  /* _RIGIDSOLVER_HPP_ */
