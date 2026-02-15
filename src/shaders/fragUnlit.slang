
struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 fragPos;
  float3 norm;
  float3 color;
  float3 cameraPos;
  float2 uv;
}

[[vk::binding(0, 1)]]
Sampler2D albedo;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  // return float4(float3(phong), 1.0);
  return float4(albedo.Sample(inVert.uv).xyz, 1.0);
  // return float4(inVert.norm / 2 + 0.5, 1.0);
}

