

enum LightType {
  POINTLIGHT,
  SPOTLIGHT,
  DIRLIGHT
};

struct VSInput {
  float3 pos;
  float3 norm;
  float2 uv;
}

struct PushConstant {
  int32_t transformIndex;
  int32_t numLights;
}

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
  float3 cameraPos;
}

struct Light {
  float3 pos;
  float3 color;
  LightType type;
  float brightness;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(0, 3)]]
StructuredBuffer<float4x4> transforms;

[[vk::push_constant]]
PushConstant push;

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 fragPos;
  float3 norm;
  float3 color;
  float3 cameraPos;
  float2 uv;
  float numLights;
}

static float ambientStrength = 0.1;
static float diffuseStrength = 1;
static float specularStrength = 0.5;
static float attConstant = 1.0;
static float attLinear = 0.09;
static float attQuadratic = 0.032;

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  output.fragPos = mul(transforms[push.transformIndex], float4(input.pos, 1.0)).xyz;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(transforms[push.transformIndex], float4(input.pos, 1.0))));
  output.color = normalize(float3(push.transformIndex));
  output.norm = input.norm;
  output.cameraPos = ubo.cameraPos;
  output.uv = input.uv;
  output.numLights = push.numLights;
  return output;
}

[[vk::binding(0, 1)]]
Sampler2D albedo;

[[vk::binding(0, 2)]]
StructuredBuffer<Light> lights;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 phong = 0;
  for (int i = 0; i < inVert.numLights; i++) {
    phong += calculatePointLight(inVert, lights[i]);
  }
  // return float4(float3(phong), 1.0);
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

