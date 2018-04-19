#pragma once

#include "platform.h"
#include "ut.h"
#include "vec_types.h"
#include "vec_ops.h"
#include "std.h"
#include "asset.h"

struct asset {
	array<u8> name;
	array<u8> data;
};

struct asset_bank {
	list<asset> assets;
	u32 code;
};

struct asset_file_builder {
	list<asset_bank> banks;
};

void import_raw(asset_file_builder* afb, const char* path, const char* name, u32 code);
void import_png(asset_file_builder* afb, const char* path, const char* name);
void import_wav(asset_file_builder* afb, const char* path, const char* name);
void import_font(asset_file_builder* afb, const char* path, const char* name);
void import_hlsl(asset_file_builder* afb, const char* path, const char* name);

bool name_copy(array<u8>& d, const char* s);
asset_bank* get_bank(asset_file_builder* afb, u32 code);
asset* new_asset(asset_file_builder* afb, u32 code, const char* name);