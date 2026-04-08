[[vk::binding(0, 0)]]
Sampler2D color;

struct VSOutput {
  float4 position : SV_Position;
  float2 uv;
};

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
  float4 fragColor = color.Sample(input.uv);
  float4 brightColor;
  float brightness = dot(fragColor.rgb, float3(0.2126, 0.7152, 0.0722));
  if (brightness > 0.5)
    brightColor = float4(fragColor.rgb, 1.0);
  else
    brightColor = float4(0.0, 0.0, 0.0, 1.0);

  return brightColor;
}
