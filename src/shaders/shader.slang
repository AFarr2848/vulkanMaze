
struct VSInput {
  float3 pos;
}

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
}
[[vk::binding(0)]]
ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(2)]]
StructuredBuffer<float4x4> buf;

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 color;
}

[shader("vertex")]
VertexOutput vertMain(VSInput input, uint instanceID: SV_InstanceID) {
  VertexOutput output;
  float4x4 model = buf[instanceID];
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
  output.color = float3(input.pos + 0.5);
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;

  return float4(color, 1.0);
}
