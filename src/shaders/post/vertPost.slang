float2 positions[3] = float2[](
    float2(-1.0, -1.0),
    float2(3.0, -1.0),
    float2(-1.0, 3.0));

uint vid : SV_VertexID;
[shader("vertex")]
float4 vertMain() : SV_Position {

  return float4(positions[vid], 0.0, 1.0);
}

