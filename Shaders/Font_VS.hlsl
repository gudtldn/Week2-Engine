struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

cbuffer constants : register(b0)
{
    matrix MVP;
    float4 CustomColor;
    uint bUseVertexColor;
}

cbuffer UUIDColor : register(b1){
    float4 UUIDColor;
}

cbuffer Depth : register(b2){
    int depth;
    int nearPlane;
    int farPlane;
}

VS_OUTPUT Font_VS(VS_INPUT input)
{
    VS_OUTPUT output;

	output.Pos = mul(float4(input.Pos.xyz, 1.0f), MVP);
    output.Tex = input.Tex;

    return output;
}