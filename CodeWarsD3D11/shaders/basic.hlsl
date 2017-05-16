

struct VOut 
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer MatrixBuffer {
	matrix worldm;
	matrix viewm;
	matrix projm;
};

VOut VShader(float4 position : POSITION, float2 tex : TEXCOORD0) {

	VOut output;
	output.pos = mul(position, worldm);
	//output.pos = mul(output.pos, viewm);
	//output.pos = mul(output.pos, projm);
	//output.pos = position;
	output.tex = tex;
	
	return output;
}

Texture2D shaderTexture;
SamplerState sampleType;

float4 PShader(float4 position : SV_POSITION, float2 tex : TEXCOORD0) : SV_TARGET
{

	return shaderTexture.Sample(sampleType, tex);
	//return float4(1, 0.2, 0.2, 1);
}