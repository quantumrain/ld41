#include "pch.h"
#include "base.h"
#include "sys.h"

// assets

struct asset_file {
	file_buf data;
};

array<asset_file> g_asset_files;

bool load_assets(const char* path) {
	asset_file* af = g_asset_files.push_back();

	if (!load_file(&af->data, path)) {
		g_asset_files.pop_back();
		return false;
	}

	return true;
}

void* find_asset(u32 code, const char* name) {
	for(auto& af : g_asset_files) {
		u8*					base	= af.data.data;
		asset_file_header*	fh		= (asset_file_header*)base;
		const char*			names	= (const char*)base;

		for(u32 bank = 0; bank < fh->bank_count; bank++) {
			asset_bank_header* bh = (asset_bank_header*)(base + fh->bank_offset) + bank;

			if (bh->code != code)
				continue;

			for(u32 entry = 0; entry < bh->entry_count; entry++) {
				asset_bank_entry*	be			= (asset_bank_entry*)(base + bh->entry_offset) + entry;
				const char*			entry_name	= (const char*)(base + be->name_offset);

				if (!strcmp(entry_name, name))
					return base + be->data_offset;
			}
		}
	}

	return 0;
}

void* find_next_asset(find_asset_data* fad, u32 code, const char** name) {
	for(; fad->file < g_asset_files.size(); fad->file++) {
		u8*					base	= g_asset_files[fad->file].data.data;
		asset_file_header*	fh		= (asset_file_header*)base;

		for(; fad->bank < fh->bank_count; fad->bank++) {
			asset_bank_header* bh = (asset_bank_header*)(base + fh->bank_offset) + fad->bank;

			if (bh->code != code)
				continue;

			while(fad->entry < bh->entry_count) {
				asset_bank_entry* be = (asset_bank_entry*)(base + bh->entry_offset) + fad->entry++;
				*name = (const char*)(base + be->name_offset);
				return base + be->data_offset;
			}

			fad->entry = 0;
		}

		fad->bank = 0;
	}

	return 0;
}

// etc

texture load_texture(const char* name) {
	raw_header* png = (raw_header*)find_asset(ASSET_PNG, name);

	if (!png) {
		debug("load_texture: can't find asset - %s", name);
		return 0;
	}

	int width, height;
	u8* data = image_from_memory((u8*)&png[1], png->size, &width, &height);

	if (!data) {
		debug("load_png: image_from_memory returned null - %s", name);
		return 0;
	}

	texture t = gpu_create_texture(width, height, gpu_format::RGBA, gpu_bind::SHADER, data); // TODO: should be srgb

	free(data);

	return t;
}

shader load_shader(input_layout* il, const char* name) {
	hlsl_header* hlsl = (hlsl_header*)find_asset(ASSET_HLSL, name);

	if (!hlsl) {
		debug("load_shader: can't find shader - %s", name);
		return 0;
	}

	u8* vs = (u8*)(hlsl + 1);
	u8* ps = vs + hlsl->vs_size;

	return gpu_create_shader(il, vs, hlsl->vs_size, ps, hlsl->ps_size);
}