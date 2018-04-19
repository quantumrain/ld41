#include "pch.h"
#include "base.h"
#include "sys.h"

font g_font;

void font_init() {
	g_font = load_font("font");
}

font load_font(const char* name) {
	font f;

	f.texture = load_texture(name);

	if (font_header* fh = (font_header*)find_asset(ASSET_FONT, name))
		memcpy(f.glyph_width, fh->glyph_width, sizeof(f.glyph_width));

	return f;
}

u8 measure_glyph(font* f, int glyph) {
	return f->glyph_width[(glyph >= 0) && (glyph < 128) ? glyph : 0];
}

float measure_string(font* f, const char* s) {
	float w = 0.0f;
	for(; *s; s++)
		w += measure_glyph(f, *s);
	return w - 1.0f; // hack
}

vec2 measure_string(const char* s) {
	return vec2(measure_string(&g_font, s), 7.0f);
}

void draw_glyph(const draw_context& dc, font* f, vec2 pos, vec2 scale, int glyph, rgba colour) {
	int sx = glyph & 15;
	int sy = glyph >> 4;

	vec2 uv0(sx / 16.0f, sy / 8.0f);
	vec2 uv1(uv0 + vec2(1.0f / 16.0f, 1.0f / 8.0f));

	vec2 fuzz(1.0f / (128.0f * 16.0f), 1.0f / (64.0f * 16.0f));

	dc.rect(pos, pos + 8.0f * scale, uv0 + fuzz, uv1 - fuzz, colour);
}

void draw_string(const draw_context& dc, vec2 pos, vec2 scale, int flags, rgba col, const char* s) {
	font* f = &g_font;

	if ((flags & TEXT_CENTRE) == TEXT_CENTRE)		pos.x -= measure_string(f, s) * 0.5f * scale.x;
	else if ((flags & TEXT_CENTRE)	== TEXT_RIGHT)	pos.x -= measure_string(f, s) * scale.x;

	if ((flags & TEXT_VCENTRE) == TEXT_VCENTRE)		pos.y -= 3.5f * scale.y;
	else if ((flags & TEXT_VCENTRE) == TEXT_BOTTOM)	pos.y -= 7.0f * scale.y;

	draw_context dcc = dc.copy().set(f->texture);

	for(; *s; s++) {
		draw_glyph(dcc, f, pos, scale, *s, col);
		pos.x += measure_glyph(f, *s) * scale.x;
	}
}

void draw_stringf(const draw_context& dc, vec2 pos, vec2 scale, int flags, rgba col, const char* fmt, ...) {
	char buf[512];

	va_list ap;
	va_start(ap, fmt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
	va_end(ap);

	draw_string(dc, pos, scale, flags, col, buf);
}