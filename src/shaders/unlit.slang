
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
  float3 cameraPos;
  float2 uv;
}

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  output.fragPos = mul(ubo.model, float4(input.pos, 1.0)).xyz;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
  output.norm = input.norm;
  output.cameraPos = ubo.cameraPos;
  output.uv = input.uv;
  return output;
}

layout(set = 1, binding = 0) uniform Sampler2D albedo;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  return albedo.Sample(inVert.uv);
  
}

