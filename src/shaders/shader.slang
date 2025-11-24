
struct VSInput{
    float3 pos;
  }

struct UniformBuffer{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
  }
ConstantBuffer<UniformBuffer> ubo;


struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 color;
  }

[shader("vertex")]
VertexOutput vertMain(VSInput input){
  VertexOutput output;
  output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
  output.color = float3(input.pos.x + 0.5, input.pos.y + 0.5, input.pos.z + 0.5);
  return output;

}


[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;
    return float4(color, 1.0);
  }
