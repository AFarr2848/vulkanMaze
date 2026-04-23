[[vk::binding(0, 0)]]
Sampler2D color;

struct VSOutput {
  float4 position : SV_Position;
  float2 uv;
};

static const float weights[5] = { 0.227027, 0.194594, 0.121621, 0.054054, 0.016216 };

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
  int width;
  int height;
  color.GetDimensions(width, height);
  float2 texelSize = 1.0 / float2(width, height);

  float3 result = color.Sample(input.uv).rgb * weights[0];

  for (int i = 1; i < 5; ++i) {
    result += color.Sample(input.uv + float2(0, 1) * texelSize * float(i)).rgb * weights[i];
    result += color.Sample(input.uv - float2(0, 1) * texelSize * float(i)).rgb * weights[i];
  }

  return float4(result, 1.0);
}
