

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 fragPos;
  float3 norm;
  float3 color;
  float3 cameraPos;
  float2 uv;
}

struct PushConstant {
  int32_t transformIndex;
  int3 lightNums;
}

enum LightType : int32_t {
  POINTLIGHT,
  SPOTLIGHT,
  DIRLIGHT
};

struct Light {
  float3 pos;
  float3 dir;
  float3 color;
  LightType type;
  float brightness;
  float param1;
  float param2;
};

static float ambientStrength = 0.1;
static float diffuseStrength = 1;
static float specularStrength = 0.5;
static float attConstant = 1.0;
static float attLinear = 0.09;
static float attQuadratic = 0.032;

[[vk::push_constant]]
PushConstant push;

[[vk::binding(0, 1)]]
Sampler2D albedo;

[[vk::binding(0, 2)]]
StructuredBuffer<Light> pLights;
[[vk::binding(1, 2)]]
StructuredBuffer<Light> dLights;
[[vk::binding(2, 2)]]
StructuredBuffer<Light> sLights;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 phong = 0;

  for (int i = 0; i < push.lightNums[0]; i++) {
    phong += calculatePointLight(inVert, pLights[i]);
  }
  for (int i = 0; i < push.lightNums[1]; i++) {
    phong += calculateDirLight(inVert, dLights[i]);
  }

  for (int i = 0; i < push.lightNums[2]; i++) {
    phong += calculateSpotLight(inVert, sLights[i]);
  }
  // return float4(float3(phong), 1.0);
  //    return float4(float3(sLights[0].param2), 1.0);
  return float4(albedo.Sample(inVert.uv).xyz * phong, 1.0);
  // return float4(inVert.norm / 2 + 0.5, 1.0);
}

float3 calculatePointLight(VertexOutput inVert, Light light) {
  float dist = distance(inVert.fragPos, light.pos);
  float attenuation = 1.0 / (attConstant + attLinear * dist + attQuadratic * (dist * dist));

  float3 lightDir = normalize(light.pos - inVert.fragPos);
  float3 norm = normalize(inVert.norm);

  float3 ambientLight = ambientStrength * light.color;

  float diff = max(dot(norm, lightDir), 0.0);
  float3 diffLight = diff * light.color * diffuseStrength;

  float3 viewDir = normalize(inVert.cameraPos - inVert.fragPos);
  float3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  float3 specLight = specularStrength * spec * light.color;

  ambientLight *= attenuation;
  diffLight *= attenuation;
  specLight *= attenuation;

  return (ambientLight + diffLight + specLight);
}

float3 calculateDirLight(VertexOutput inVert, Light light) {
  float3 lightDir = normalize(light.dir);
  float3 norm = normalize(inVert.norm);

  float3 ambientLight = ambientStrength * light.color;

  float diff = max(dot(norm, lightDir), 0.0);
  float3 diffLight = diff * light.color * diffuseStrength;

  float3 viewDir = normalize(inVert.cameraPos - inVert.fragPos);
  float3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  float3 specLight = specularStrength * spec * light.color;

  return (ambientLight + diffLight + specLight);
}

float3 calculateSpotLight(VertexOutput inVert, Light light) {

  float3 norm = normalize(inVert.norm);

  float3 lightDir = normalize(light.pos - inVert.fragPos);
  float3 viewDir = normalize(inVert.cameraPos - inVert.fragPos);
  // diffuse shading

  float diff = max(dot(norm, lightDir), 0.0);
  // specular shading
  float3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

  // attenuation
  float distance = length(light.pos - inVert.fragPos);
  float attenuation = 1.0 / (attConstant + attLinear * distance + attQuadratic * (distance * distance));
  // spotlight intensity
  float theta = dot(lightDir, normalize(-light.dir));
  float epsilon = light.param1 - light.param2;
  float intensity = clamp((theta - light.param2) / epsilon, 0.0, 1.0);
  // combine results
  float3 ambient = light.color;
  float3 diffuse = light.color * diff;
  float3 specular = light.color * spec;
  ambient *= attenuation * intensity;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;
  return (ambient + diffuse + specular);
}
