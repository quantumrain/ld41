VS_INPUT_XYZW_UV

struct VS_OUTPUT {
	float4 xyzw	: SV_POSITION;
	float2 uv	: TEXCOORD0;
};

Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

VS_OUTPUT vs_main(VS_INPUT v) {
	VS_OUTPUT output;

	output.xyzw	= v.xyzw;
	output.uv	= v.uv;

	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET {
	return t0.Sample(s0, input.uv);
}