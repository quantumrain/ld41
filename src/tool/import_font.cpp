#include "pch.h"
#include "tool.h"

void import_font(asset_file_builder* afb, const char* path, const char* name) {
	// load related font png

	char png_path[MAX_PATH];
	_snprintf_s(png_path, MAX_PATH, _TRUNCATE, "%.*s.png", sz_find_last_or_end(path, '.') - path, path);

	file_buf f;

	if (!load_file(&f, png_path)) {
		debug("import_font: failed to load - %s", png_path);
		return;
	}

	int width, height;
	u8* data = image_from_memory(f.data, f.size, &width, &height);

	if (!data) {
		debug("import_font: png failed to parse - %s", png_path);
		return;
	}

	// generate char widths

	font_header fh;

	int cw = width / 16;
	int ch = height / 8;

	for(int cy = 0, glyph = 0; cy < 8; cy++) {
		for(int cx = 0; cx < 16; cx++, glyph++) {
			int ox = cx * cw;
			int oy = cy * ch;

			int x = cw - 1;

			for(; x >= 0; x--) {
				u32* p = (u32*)data + (oy * width) + (ox + x);

				for(int y = 0; y < ch; y++, p += width) {
					if (p[0]) goto done;
				}
			}

		done:;
			fh.glyph_width[glyph] = x + 2;
		}
	}

	fh.glyph_width[' '] = 4; // TODO: don't hardcode this

	free(data);

	// write asset

	asset* a = new_asset(afb, ASSET_FONT, name);

	if (!a) {
		debug("import_font: failed to allocate asset - %s", path);
		return;
	}

	u32 total_size = sizeof(font_header);

	if (!a->data.set_size(total_size)) {
		debug("import_font: failed to allocate asset data - %s", path);
		return;
	}

	*(font_header*)a->data.begin() = fh;
}