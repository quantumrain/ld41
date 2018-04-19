#include "pch.h"
#include "tool.h"

#include <initguid.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

// todo: use directive in file rather than macros (as the macros suck)
// e.g. @vs_input xyzw_uv ?

const char* vs_input_xyzw_uv = 
	"struct VS_INPUT {"
		"float4 xyzw : POSITION;"
		"float2 uv : TEXCOORD0;"
	"};";

const char* vs_input_xyz_uv_rgba = 
	"struct VS_INPUT {"
		"float3 xyz : POSITION;"
		"float2 uv : TEXCOORD0;"
		"float4 rgba : COLOR0;"
	"};";

void import_hlsl(asset_file_builder* afb, const char* path, const char* name) {

	wchar_t wpath[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);

	D3D_SHADER_MACRO macros[] = {
		{ "VS_INPUT_XYZW_UV",		vs_input_xyzw_uv },
		{ "VS_INPUT_XYZ_UV_RGBA",	vs_input_xyz_uv_rgba },
		{ 0 }
	};

	ID3DBlob* vs_code	= 0;
	ID3DBlob* vs_errors	= 0;
	ID3DBlob* ps_code	= 0;
	ID3DBlob* ps_errors	= 0;

	HRESULT vs_hr = D3DCompileFromFile(wpath, macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vs_code, &vs_errors);
	HRESULT ps_hr = D3DCompileFromFile(wpath, macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &ps_code, &ps_errors);

	if (vs_errors) {
		debug("%s: import_hlsl: compiling VERTEX shader", path);
		debug("%.*s", vs_errors->GetBufferSize(), vs_errors->GetBufferPointer());
		vs_errors->Release();
	}

	if (ps_errors) {
		debug("%s: import_hlsl: compiling PIXEL shader", path);
		debug("%.*s", ps_errors->GetBufferSize(), ps_errors->GetBufferPointer());
		ps_errors->Release();
	}

	if (FAILED(vs_hr) || FAILED(ps_hr) || !vs_code || !ps_code) {
		debug("import_hlsl: compile failed - %s", path);
		if (vs_code) vs_code->Release();
		if (ps_code) ps_code->Release();
		return;
	}

	asset* a = new_asset(afb, ASSET_HLSL, name);

	if (!a) {
		debug("import_hlsl: failed to allocate asset - %s", path);
		vs_code->Release();
		ps_code->Release();
		return;
	}

	u32 vs_size = vs_code->GetBufferSize();
	u32 ps_size = ps_code->GetBufferSize();

	u32 total_size = sizeof(hlsl_header) + vs_size + ps_size;

	if (!a->data.set_size(total_size)) {
		debug("import_hlsl: failed to allocate asset data - %s", path);
		vs_code->Release();
		ps_code->Release();
		return;
	}

	hlsl_header*	h	= (hlsl_header*)a->data.begin();
	u8*				vs	= a->data.begin() + sizeof(hlsl_header);
	u8*				ps	= a->data.begin() + sizeof(hlsl_header) + vs_size;

	h->vs_size = vs_size;
	h->ps_size = ps_size;

	memcpy(vs, vs_code->GetBufferPointer(), vs_size);
	memcpy(ps, ps_code->GetBufferPointer(), ps_size);

	vs_code->Release();
	ps_code->Release();
}