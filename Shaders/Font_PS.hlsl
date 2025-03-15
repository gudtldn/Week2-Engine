Texture2D fontAtlas : register(t0);
SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

float4 Font_PS(VS_OUTPUT input) : SV_TARGET
{
	float4 sampledColor = fontAtlas.Sample(samLinear, input.Tex);
	
	float threshold = 0.3f;
	float colorSum = sampledColor.r + sampledColor.g + sampledColor.b;
	
	return colorSum < threshold ? float4(.0f, .0f, .0f, .0f) : sampledColor;
}