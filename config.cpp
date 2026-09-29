#include "src/config.hpp"

void configScene()
{
  // =================================================================================

  // LIGHTS
  // Note: Maximum of *3 lights* only are supported in the current shader implementation

  auto light1 = create_light();
  light1->position = glm::vec3(0, 0.5, 1);
  // light1->color = glm::vec3(0.5, 0.5, 1);
  // light1->perspec_angle = 90;

  // auto light2 = create_light();
  // light2->position = glm::vec3(-1, -1, 1.5);
  // light2->color = glm::vec3(1, 0.5, 0.5);
  // light2->intensity = 0.5;

  
  
  // =================================================================================
  
  // MESHES

  auto planeMesh = create_mesh("plane");
  planeMesh->addPlane();

  auto cubeMesh = create_mesh("cube");
  cubeMesh->addBox(.1f, .1f, .1f);

  auto sphereMesh = create_mesh("sphere");
  sphereMesh->addSphere(0.1f, 10);

  auto polyMesh = create_mesh("polyhedron", "data/sphere.off", true);
  polyMesh->scaleMesh(0.1f);

  // auto monkeyMesh = create_mesh("monkey", "data/monkey.off", true);
  // monkeyMesh->scaleMesh(0.1f);

  auto customMesh = create_mesh("custom");
  std::vector<std::vector<int> > shape1 = {
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,0,0,0,1},
  };
  float voxel_w = .2f, voxel_h = .1f, voxel_d = .6f;
  customMesh->meshFromMatrix(shape1, voxel_w, voxel_h, voxel_d);
  
  // auto customMesh2 = create_mesh("custom2");
  // std::vector<std::vector<int> > shape2 = {
  //   {1,0,0,0,1},
  //   {1,1,1,1,1},
  //   {0,0,1,0,1},
  // };
  // float voxel_w2 = .05f, voxel_h2 = .05f, voxel_d2 = .05f;
  // customMesh2->meshFromMatrix(shape2, voxel_w2, voxel_h2, voxel_d2);

  // =================================================================================

  // MATERIALS

  auto standardSurface = create_material("standardSurface");
  standardSurface->albedo = glm::vec3(1, 0.71, 0.29);
  standardSurface->restitution = 0.3f;
  // standardSurface->damping=0.01f;

  auto basicSurface = create_material("basicSurface", "data/color.png", "data/normal.png");
  basicSurface->restitution = 0.3f;
  // basicSurface->damping=0.01f;

  auto dieSurface = create_material("dieSurface", "data/dice.png");

  // auto astroSurface = create_material("astroSurface", "data/earth.jpg");
  // astroSurface->specK = 3.0;
  // astroSurface->specA = 3.0;


  // =================================================================================

  // GRAPHICAL OBJECTS

  auto backWallObj = create_graphical_object("back_wall", planeMesh, basicSurface);
  backWallObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -1));

  auto floorObj = create_graphical_object("floor", planeMesh, basicSurface);
  floorObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0, -1, 0))*
  glm::rotate(glm::mat4(1.0), (float)(-0.5f*M_PI), glm::vec3(1.0, 0.0, 0.0));

  auto leftWallObj = create_graphical_object("left_wall", planeMesh, basicSurface);
  leftWallObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(-1, 0, 0))*
  glm::rotate(glm::mat4(1.0), (float)(0.5f*M_PI), glm::vec3(0.0, 1.0, 0.0));

  auto rightWallObj = create_graphical_object("right_wall", planeMesh, basicSurface);
  rightWallObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(1, 0, 0))*
  glm::rotate(glm::mat4(1.0), (float)(-0.5f*M_PI), glm::vec3(0.0, 1.0, 0.0));

  auto dieObj1 = create_graphical_object("die_1", cubeMesh, dieSurface, true);
  dieObj1->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(-0.5, 0, 0));
  dieObj1->addBoxBody(.1f, .1f, .1f);
  dieObj1->setBodyCenter(Vec3f(-0.5, 0, 0));

  // auto sphereObj = create_graphical_object("sphere", sphereMesh, astroSurface, true);
  // sphereObj->addRoundBody(sphereMesh, .1f);
  // sphereObj->setBodyCenter(Vec3f(0.5, 0, 0));

  // auto monkeyObj = create_graphical_object("monkey", monkeyMesh, standardSurface);
  // monkeyObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0, 0));
  // monkeyObj->addBoxedBody(monkeyMesh, .1f,.1f,.1f);

  auto customObj = create_graphical_object("custom", customMesh, standardSurface);
  customObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0, -0.5, 0));
  customObj->addBoxedBody(customMesh, shape1.size()*voxel_w, shape1[0].size()*voxel_h, voxel_d);
  customObj->setBodyCenter(Vec3f(0, -0.7, 0));
  
  auto polyObj = create_graphical_object("polyhedron", polyMesh, standardSurface, true);
  polyObj->modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0, 0.5, 0));
  polyObj->addRoundBody(polyMesh, .1f);
  polyObj->setBodyCenter(Vec3f(0, 0.5, 0));

  // auto customObj2 = create_graphical_object("custom2", customMesh2, standardSurface, true);
  // customObj2->addBoxedBody(customMesh2, shape2[0].size()*voxel_w2, shape2.size()*voxel_h2, voxel_d2);


  // =================================================================================

}

