#version 330 core            // minimal GL version support expected from the GPU

struct LightSource {
  vec3 position;
  vec3 color;
  float intensity;
  int isActive;
};

int numberOfLights = 3;
uniform LightSource lightSources[3];

// Shadow maps
uniform sampler2D shadowMaps[3];
uniform mat4 lightDepthMVP[3];

struct Material {
  vec3 albedo;
  sampler2D normalMap;
  int useNormalMap;
  sampler2D albedoMap;
  int useAlbedoMap;
  float specK;
  float specA;
};
uniform Material material;

uniform vec3 camPos;
uniform mat4 modelMat;

in vec3 fPositionModel;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorOut; // shader output: the color response attached to this fragment

float pi = 3.1415927;

float computeShadow(int index, float bias) {
  // Shadow calculation: transform fragment into light clip space
  vec4 posLight = lightDepthMVP[index] * modelMat * vec4(fPositionModel, 1.0);
  vec3 projCoords = posLight.xyz / posLight.w; // NDC in [-1,1]
  projCoords = projCoords * 0.5 + 0.5; // to [0,1] for texture lookup

  // default: lit
  float shadow = 1.0;

  // check if inside shadow map frustum
  if(projCoords.x >= 0.0 && projCoords.x <= 1.0 && projCoords.y >= 0.0 && projCoords.y <= 1.0 && projCoords.z >= 0.0 && projCoords.z <= 1.0) {
    // depth from light's POV
    float closestDepth = 1.0;
    if(index == 0) {
      closestDepth = texture(shadowMaps[0], projCoords.xy).r;
    } else if(index == 1) {
      closestDepth = texture(shadowMaps[1], projCoords.xy).r;
    } else if(index == 2) {
      closestDepth = texture(shadowMaps[2], projCoords.xy).r;
    }

    if(projCoords.z > closestDepth + bias)
      shadow = 0.0;
  }

  return shadow;
}


void main() {
  vec3 ns = normalize(fNormal);
  if(material.useNormalMap == 1) {
    vec3 dp1 = dFdx(fPosition);
    vec3 dp2 = dFdy(fPosition);
    vec2 duv1 = dFdx(fTexCoord);
    vec2 duv2 = dFdy(fTexCoord);

    vec3 t = normalize(duv2.y * dp1 - duv1.y * dp2);
    vec3 b = normalize(-duv2.x * dp1 + duv1.x * dp2);

    // Sample normal map and remap from [0,1] to [-1,1]
    vec3 nSample = texture(material.normalMap, fTexCoord).rgb;
    nSample = nSample * 2.0 - 1.0;

    // Build TBN matrix and transform normal map vector into world/view space
    mat3 TBN = mat3(t, b, ns);
    ns = normalize(TBN*nSample);
  }
  vec3 wo = normalize(camPos - fPosition); // unit vector pointing to the camera

  // determine albedo (either uniform material or texture)
  vec3 baseAlbedo = material.albedo;
  if(material.useAlbedoMap == 1) {
    baseAlbedo = texture(material.albedoMap, fTexCoord).rgb;
  }

  vec3 radiance = vec3(0, 0, 0);
  vec3 v = normalize(camPos - fPosition);
  for(int i=0; i<numberOfLights; ++i) {
    LightSource a_light = lightSources[i];
    if(a_light.isActive == 1) { // consider active lights only
      vec3 wi = normalize(a_light.position - fPosition); // unit vector pointing to the light
      vec3 Li = a_light.color*a_light.intensity;

      // bias to reduce shadow acne (can be tuned)
      float bias = max(0.005 * (1.0 - dot(ns, wi)), 0.005);
      float shadow = computeShadow(i, bias);

      // Use mapped normal and apply shadow
      vec3 r = reflect(-wi, ns);
      vec3 specular = material.specK * pow(max(dot(v, r), 0.0), material.specA) * baseAlbedo;
      radiance += Li * (baseAlbedo + specular) * max(dot(ns, wi), 0) * shadow;
    }
  }

  colorOut = vec4(radiance, 1.0); // build an RGBA value from an RGB one
}
