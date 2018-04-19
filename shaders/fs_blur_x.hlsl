VS_INPUT_XYZW_UV

struct VS_OUTPUT {
	float4 xyzw	: SV_POSITION;
	float2 uv	: TEXCOORD0;
};

float2 texel_size : register(c0);

Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

VS_OUTPUT vs_main(VS_INPUT v) {
	VS_OUTPUT output;

	output.xyzw	= v.xyzw;
	output.uv	= v.uv;

	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET {
	float4 r;

	r  = t0.Sample(s0, float2(input.uv.x,						input.uv.y)) * 0.319f;
	r += t0.Sample(s0, float2(input.uv.x - texel_size.x,		input.uv.y)) * 0.232f;
	r += t0.Sample(s0, float2(input.uv.x + texel_size.x,		input.uv.y)) * 0.232f;
	r += t0.Sample(s0, float2(input.uv.x - 2.0f * texel_size.x,	input.uv.y)) * 0.089f;
	r += t0.Sample(s0, float2(input.uv.x + 2.0f * texel_size.x,	input.uv.y)) * 0.089f;
	r += t0.Sample(s0, float2(input.uv.x - 3.0f * texel_size.x,	input.uv.y)) * 0.018f;
	r += t0.Sample(s0, float2(input.uv.x + 3.0f * texel_size.x,	input.uv.y)) * 0.018f;
	r += t0.Sample(s0, float2(input.uv.x - 4.0f * texel_size.x,	input.uv.y)) * 0.002f;
	r += t0.Sample(s0, float2(input.uv.x + 4.0f * texel_size.x,	input.uv.y)) * 0.002f;

	return r;
}