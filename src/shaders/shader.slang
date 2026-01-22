
struct VSInput {
  float3 pos;
  float3 norm;
  float2 uv;
}

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
  float3 cameraPos;
}

[[vk::binding(0)]]
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 fragPos;
  float3 norm;
  float3 color;
  float3 cameraPos;
  float2 uv;
}

struct PointLight {
  float3 pos;
  float3 color;
  float3 brightness;
}

static PointLight pointLight = PointLight(float3(1.5), float3(2.0, 2.0, 2.0), 1.0);
static float ambientStrength = 0.3;
static float diffuseStrength = 0.3;
static float specularStrength = 0.8;

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  output.fragPos = mul(ubo.model, float4(input.pos, 1.0)).xyz;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
  output.color = normalize(float3(-output.sv_position.z));
  output.norm = input.norm;
  output.cameraPos = ubo.cameraPos;
  output.uv = input.uv;
  return output;
}

layout(set = 1, binding = 0) uniform Sampler2D albedo;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;
  float3 phong = calculatePointLight(inVert, pointLight);

  return float4((albedo.Sample(inVert.uv).xyz * phong), 1.0);
}

float3 calculatePointLight(VertexOutput inVert, PointLight light) {
  float3 lightDir = normalize(light.pos - inVert.fragPos);

  float3 ambientLight = ambientStrength * light.color;

  float diff = max(dot(inVert.norm, lightDir), 0.0);
  float3 diffLight = diff * light.color;

  float3 viewDir = normalize(inVert.cameraPos - inVert.fragPos);
  float3 reflectDir = reflect(-lightDir, inVert.norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  float3 specLight = specularStrength * spec * light.color;

  return (ambientLight + diffLight + specLight) * float3(0.1, 0.1, 0.1);
}

