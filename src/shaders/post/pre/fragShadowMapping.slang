struct VertexOutput {
  float4 sv_position : SV_Position;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
}

