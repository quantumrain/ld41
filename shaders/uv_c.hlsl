VS_INPUT_XYZ_UV_RGBA

struct VS_OUTPUT {
	float4 xyzw	: SV_POSITION;
	float2 uv	: TEXCOORD0;
	float4 rgba	: COLOR0;
};

float4x4 proj_view : register(c0);

Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

/*cbuffer cb_view : register(b0) {
	float4x4 proj_view;
}*/

VS_OUTPUT vs_main(VS_INPUT v) {
	VS_OUTPUT output;

	output.xyzw	= mul(proj_view, float4(v.xyz, 1.0f));
	output.uv	= v.uv;
	output.rgba	= v.rgba;

	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET {
	return t0.Sample(s0, input.uv) * input.rgba;
}