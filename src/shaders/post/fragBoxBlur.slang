[[vk::binding(0, 0)]]
Sampler2D sceneColor;

struct VSOutput {
  float4 position : SV_Position;
  float2 uv;
};

static const int range = 3;

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
  int width;
  int height;
  sceneColor.GetDimensions(width, height);
  float2 texelSize = 1.0 / float2(width, height);

  float4 texSum = float4(0);
  float weightSum = 0;

  for (int y = -range; y <= range; y++) {
    for (int x = -range; x <= range; x++) {
      float2 offset = float2(x, y) * texelSize;
      texSum += sceneColor.Sample(input.uv + offset);
      weightSum += 1.0;
    }
  }

  return texSum / weightSum;
}
