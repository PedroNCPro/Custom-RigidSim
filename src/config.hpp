#ifndef CONFIG_H
#define CONFIG_H

#include <glad/gl.h>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <map>

#include "ShaderProgram.h"
#include "Mesh.h"
#include "RigidSolver.hpp"

class FboShadowMap {
public:
  GLuint getTextureId() const { return _depthMapTexture; }

  bool allocate(unsigned int width=1024, unsigned int height=768)
  {
    glGenFramebuffers(1, &_depthMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFbo);

    _depthMapTextureWidth = width;
    _depthMapTextureHeight = height;

    // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
    glGenTextures(1, &_depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, _depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthMapTexture, 0);

    glDrawBuffer(GL_NONE);      // No color buffers are written.

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return true;
    } else {
      std::cout << "PROBLEM IN FBO FboShadowMap::allocate(): FBO NOT successfully created" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      return false;
    }
  }

  void bindFbo()
  {
    glViewport(0, 0, _depthMapTextureWidth, _depthMapTextureHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  void free() { glDeleteFramebuffers(1, &_depthMapFbo); }

  void savePpmFile(std::string const &filename)
  {
    std::ofstream output_image(filename.c_str());

    // READ THE PIXELS VALUES from FBO AND SAVE TO A .PPM FILE
    int i, j, k;
    float *pixels = new float[_depthMapTextureWidth*_depthMapTextureHeight];

    // READ THE CONTENT FROM THE FBO
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, _depthMapTextureWidth, _depthMapTextureHeight, GL_DEPTH_COMPONENT , GL_FLOAT, pixels);

    output_image << "P3" << std::endl;
    output_image << _depthMapTextureWidth << " " << _depthMapTextureHeight << std::endl;
    output_image << "255" << std::endl;

    k = 0;
    for(i=0; i<_depthMapTextureWidth; ++i) {
      for(j=0; j<_depthMapTextureHeight; ++j) {
        output_image <<
          static_cast<unsigned int>(255*pixels[k]) << " " <<
          static_cast<unsigned int>(255*pixels[k]) << " " <<
          static_cast<unsigned int>(255*pixels[k]) << " ";
        k = k+1;
      }
      output_image << std::endl;
    }
    delete [] pixels;
    output_image.close();
  }

private:
  GLuint _depthMapFbo;
  GLuint _depthMapTexture;
  unsigned int _depthMapTextureWidth;
  unsigned int _depthMapTextureHeight;
};

struct Light {
  FboShadowMap shadowMap;
  glm::mat4 depthMVP;
  unsigned int shadowMapTexOnGPU;

  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
  float intensity = 1.0f;
  float perspec_angle = 0.0f;

  void setupCameraForShadowMapping(
    std::shared_ptr<ShaderProgram> shader_shadow_map_Ptr,
    const glm::vec3 scene_center,
    const float scene_radius)
  {

    // Compute the light's view and projection matrices and store the combined
    // projection*view into depthMVP (model will be applied per-object).
    // Use a perspective or orthographic projection from the light position looking at the scene center.
    float nearP = std::max(0.01f * scene_radius, 0.001f);
    float farP = std::max(10.0f * scene_radius, nearP + 1.0f);
    glm::mat4 proj;
    
    if(perspec_angle > 0.0f){
      // Perspective: Choose a 90-degree FOV and near/far based on scene radius.
      float fov = glm::radians(perspec_angle);
      float aspect = 1.0f; // shadow map is square (allocated w x h)
      proj = glm::perspective(fov, aspect, nearP, farP);
    }
    else {
      // Orthographic: Choose ortho bounds based on the scene radius so the shadow map covers the scene.
      float halfSize = scene_radius * 2.0f;
      proj = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, nearP, farP);
    }
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(position, scene_center, up);

    depthMVP = proj * view;
  }

  void allocateShadowMapFbo(unsigned int w=800, unsigned int h=600)
  {
    shadowMap.allocate(w, h);
  }
  void bindShadowMap()
  {
    shadowMap.bindFbo();
  }
};

struct Material {
  glm::vec3 albedo = glm::vec3(0, 0, 0);
  GLuint albedoId = -1;
  unsigned int albedoMap = -1;
  int useAlbedoMap = 0;
  GLuint normalId = -1;
  unsigned int normalMap = -1;
  int useNormalMap = 0;
  float specK = 0.5f;
  float specA = 30.0f;

  float restitution = 1.0f;
  float damping = 0.0f;
};

struct GraphicalObject {
  std::shared_ptr<Mesh> mesh = nullptr;
  std::shared_ptr<Material> material = nullptr;
  glm::mat4 modelMat = glm::mat4(1.0);
  std::shared_ptr<BodyAttributes> rigidAtt = nullptr; // if this object is associated with a rigid body, store its attributes here for easy access

  void render(std::shared_ptr<ShaderProgram>& mainShader) const
  {
    mainShader->set("material.albedo", material->albedo);
    mainShader->set("material.albedoMap", (int)material->albedoMap);
    mainShader->set("material.useAlbedoMap", material->useAlbedoMap);
    mainShader->set("material.normalMap", (int)material->normalMap);
    mainShader->set("material.useNormalMap", material->useNormalMap);
    mainShader->set("material.specK", material->specK);
    mainShader->set("material.specA", material->specA);
    mainShader->set("modelMat", modelMat);
    mainShader->set("normMat", glm::mat3(glm::inverseTranspose(modelMat)));
    if(mesh) mesh->render();
  }

  void render_shadows(Light light, std::shared_ptr<ShaderProgram>& shadowMapShader) const
  {
    shadowMapShader->set("depthMVP", light.depthMVP * modelMat);
    if(mesh) mesh->render();
  }

  void setBodyCenter(Vec3f newCenter)
  {
    if(rigidAtt) {
      rigidAtt->X0 = newCenter;
      rigidAtt->reset();
      modelMat = rigidAtt->worldMat();
    }
  }

  void addBoxBody(float w, float h, float d)
  {
    rigidAtt = std::make_shared<Box>(w, h, d);
    modelMat = rigidAtt->worldMat();
  }

  void addBoxedBody(const std::shared_ptr<Mesh> &mesh, float w, float h, float d)
  {
    rigidAtt = std::make_shared<BoxedBody>(mesh, w, h, d);
    modelMat = rigidAtt->worldMat();
  }

  void addRoundBody(const std::shared_ptr<Mesh> &mesh, float radius)
  {
    rigidAtt = std::make_shared<RoundBody>(mesh, radius);
    modelMat = rigidAtt->worldMat();
  }

};

std::shared_ptr<Mesh> create_mesh(const std::string &name, const std::string &path = "", bool has_subdiv = false);
std::shared_ptr<Material> create_material(const std::string &name, const std::string &albedoMapPath = "", const std::string &normalMapPath = "");
std::shared_ptr<GraphicalObject> create_graphical_object(const std::string &name, const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material, bool isBody = false);
std::shared_ptr<Light> create_light(unsigned int shadow_map_width=2000, unsigned int shadow_map_height=2000);
void configScene();


#endif