
struct VSInput {
  float3 pos;
  float3 norm;
  float2 uv;
}

struct PushConstant {
  float4x4 lightTransform;
  int32_t transformIndex;
}

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
  float3 cameraPos;
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

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(1, 0)]]
StructuredBuffer<float4x4> transforms;

[[vk::push_constant]]
PushConstant push;

struct VertexOutput {
  float4 sv_position : SV_Position;
}

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  float4x4 model = transforms[push.transformIndex];
  output.sv_position = mul(push.lightTransform, mul(model, float4(input.pos, 1.0)));
  return output;
}
