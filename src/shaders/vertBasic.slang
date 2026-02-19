

struct VSInput {
  float3 pos;
  float3 norm;
  float2 uv;
}

struct PushConstant {
  int32_t transformIndex;
  int3 lightNums;
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
}

[shader("vertex")]
VertexOutput vertMain(VSInput input) {
  VertexOutput output;
  output.fragPos = mul(transforms[push.transformIndex], float4(input.pos, 1.0)).xyz;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(transforms[push.transformIndex], float4(input.pos, 1.0))));
  output.color = normalize(float3(push.transformIndex));
  output.norm = input.norm;
  output.cameraPos = ubo.cameraPos;
  output.uv = input.uv;
  return output;
}
