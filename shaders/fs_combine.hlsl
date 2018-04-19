VS_INPUT_XYZW_UV

struct VS_OUTPUT {
	float4 xyzw	: SV_POSITION;
	float2 uv	: TEXCOORD0;
};

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
Texture2D t4 : register(t4);
Texture2D t5 : register(t5);

SamplerState s0 : register(s0);
SamplerState s1 : register(s1);

float4 seed : register(c0);

float noise(float2 p) {
	float2 a = seed.xy + p;
	float b = frac(a.x / (3.0f + sin(a.x) + sin(a.y)));
	return b * b;
}

VS_OUTPUT vs_main(VS_INPUT v) {
	VS_OUTPUT output;

	output.xyzw	= v.xyzw;
	output.uv	= v.uv;

	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET {
	float4 colour = t0.Sample(s0, input.uv);
	float4 bloom;

	bloom = t1.Sample(s1, input.uv);
	bloom += t2.Sample(s1, input.uv);
	bloom += t3.Sample(s1, input.uv);
	bloom += t4.Sample(s1, input.uv);
	bloom += t5.Sample(s1, input.uv);

	float4 r = colour + bloom * bloom * 0.1f;

	float4 noise_f = (1.0f - saturate(r)) * 0.1f;

	r *= (1.0f - noise_f) + (noise(input.uv * 1000.0f) * 2.0f) * noise_f;

	return r;
}