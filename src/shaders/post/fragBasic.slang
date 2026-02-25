
[[vk::binding(0, 0)]]
Sampler2D sceneColor;

struct VSOutput {
  float4 position : SV_Position;
  float2 uv;
};

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
  // float4 ahfafhfdh = float4(input.position.x, input.position.y, sceneColor.Sample(0, 0).x, 1.0);
  return sceneColor.Sample(input.uv);
  // return float4(1, 0, 0, 1);
}
