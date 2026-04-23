
[[vk::binding(0, 0)]]
Sampler2D color;
[[vk::binding(1, 0)]]
Sampler2D image2;

struct VSOutput {
  float4 position : SV_Position;
  float2 uv;
};

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
  return color.Sample(input.uv) + image2.Sample(input.uv);
}
