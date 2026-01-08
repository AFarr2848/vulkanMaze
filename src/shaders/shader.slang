
struct VSInput {
  float3 pos;
  float3 norm;
}

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
}

[[vk::binding(0)]]
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 fragPos;
  float3 norm;
  float3 color;
}

struct PointLight {
  float3 pos;
  float3 color;
  float3 brightness;
}

static float ambientStrength = 0.1;
static PointLight pointLight = PointLight(float3(1.5), float3(1.0, 0.0, 1.0), 1.0);

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  output.fragPos = mul(ubo.model, float4(input.pos, 1.0)).xyz;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
  output.color = normalize(float3(-output.sv_position.z));
  output.norm = input.norm;
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;
  calculatePointLight(inVert, pointLight);

  return float4(normalize(inVert.norm + 0.5), 1.0);
}

void calculatePointLight(VertexOutput inVert, PointLight light) {
  float3 ambientLight = ambientStrength * light.color;
  float3 lightDir = normalize(light.pos - inVert.fragPos);
  float diff = max(dot(inVert.norm, lightDir), 0.0);
}
