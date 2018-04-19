#pragma once

// render state

extern pipeline g_pipeline_sprite;
extern pipeline g_pipeline_bloom_reduce;
extern pipeline g_pipeline_bloom_blur_x;
extern pipeline g_pipeline_bloom_blur_y;
extern pipeline g_pipeline_combine;

extern texture g_texture_white;
extern texture g_sheet;

void init_render_state();

// draw

struct v_xyz_uv_rgba {
	float x, y, z;
	float u, v;
	float r, g, b, a;
};

struct v_xyz_n_uv_rgba {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float r, g, b, a;
};

struct v_xyzw_uv {
	float x, y, z, w;
	float u, v;
};

struct draw_buffer {
	draw_buffer();
	~draw_buffer();

	bool init(int vertex_stride, int max_vertices, int max_buffers);
	void destroy();

	void reset();
	u8* alloc(int count);

	void flush();

	vertex_buffer* _vb;

	u8* _vertices;

	int _num_vertices;
	int _max_vertices;
	int _vertex_stride;

	int _buffer_index;
	int _max_buffers;
};

struct draw_list {
	struct batch { texture t; u16 count; };

	int         _max_batches;
	int         _num_batches;
	batch*      _batches;

	draw_buffer _vertices;

	draw_list();
	~draw_list();

	bool init(int max_triangles, int max_batches);
	void destroy();

	void reset();
	v_xyz_uv_rgba* alloc(texture t, int count);

	void render();
};

struct draw_context {
	draw_context(draw_list& dl) : _dl(&dl), _texture(g_texture_white) { }

	draw_context copy() const { return *this; }

	draw_context& set(texture t)          { _texture   = t; return *this; }
	draw_context& set(const rgba& c)      { _colour    = c; return *this; }
	draw_context& set(const transform& t) { _transform = t; return *this; }

	draw_context& push(const rgba& c)       { _colour    = _colour * c;    return *this; }
	draw_context& push(const transform& t)  { _transform = _transform * t; return *this; }

	draw_context& translate(vec2 p)           { return push(::translate(vec3(p, 0.0f))); }
	draw_context& translate(float x, float y) { return push(::translate(vec3(x, y, 0.0f))); }
	draw_context& rotate_z(float x)           { return push(::rotate_z(x)); }
	draw_context& scale(float x)              { return push(::scale(vec3(x, x, 1.0f))); }
	draw_context& scale_xy(float x, float y)  { return push(::scale(vec3(x, y, 1.0f))); }
	draw_context& colour(const rgba& c)       { return push(c); }

	void line(vec2 p0, vec2 p1, float w, rgba c0, rgba c1) const;
	void line_ex(vec2 p0, vec2 p1, float w0, float w1, rgba c0, rgba c1) const;
	void line(vec2 p0, vec2 p1, float w, rgba c) const;

	void line_with_cap(vec2 p0, vec2 p1, float w, rgba c0, rgba c1) const;

	void tri(vec2 p0, vec2 p1, vec2 p2, rgba c) const;
	void tri(vec2 p0, vec2 p1, vec2 p2, rgba c0, rgba c1, rgba c2) const;
	void tri(vec2 p0, vec2 p1, vec2 p2, vec2 uv0, vec2 uv1, vec2 uv2, rgba c0, rgba c1, rgba c2) const;

	void quad(vec2 p0, vec2 p1, vec2 p2, vec2 p3, rgba c) const;
	void quad(vec2 p0, vec2 p1, vec2 p2, vec2 p3, rgba c0, rgba c1, rgba c2, rgba c3) const;
	void quad(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 uv0, vec2 uv1, vec2 uv2, vec2 uv3, rgba c0, rgba c1, rgba c2, rgba c3) const;

	void rect(vec2 p0, vec2 p1, rgba c) const;
	void rect(vec2 p0, vec2 p1, rgba c0, rgba c1, rgba c2, rgba c3) const;
	void rect(vec2 p0, vec2 p1, vec2 uv0, vec2 uv1, rgba c0, rgba c1, rgba c2, rgba c3) const;
	void rect(vec2 p0, vec2 p1, vec2 uv0, vec2 uv1, rgba c) const;

	void rect_outline(vec2 p0, vec2 p1, float line_width, rgba c) const;

	void fill(int count, const vec2* p, rgba c) const;
	void outline(int count, const vec2* p, float line_width, rgba c) const;
	void outline_open(int count, const vec2* p, float line_width, rgba c) const;

	void fill(std::initializer_list<vec2> l, rgba c) const { return fill((int)l.size(), l.begin(), c); }
	void outline(std::initializer_list<vec2> l, float line_width, rgba c) const { return outline((int)l.size(), l.begin(), line_width, c); }
	void outline_open(std::initializer_list<vec2> l, float line_width, rgba c) const { return outline_open((int)l.size(), l.begin(), line_width, c); }

	void shape(vec2 p, int vcount, float radius, float rot, rgba c) const;
	void shape_outline(vec2 p, int vcount, float radius, float rot, float w, rgba c) const;

	draw_list*	_dl;
	texture		_texture;
	rgba		_colour;
	transform	_transform;
};

enum draw_tile_flags {
	DT_ROT_0   = 0,
	DT_ROT_90  = 1,
	DT_ROT_180 = 2,
	DT_ROT_270 = 3,
	DT_FLIP_X  = 4,
	DT_FLIP_Y  = 8,
	DT_ALT_TRI = 16
};

void draw_tile(const draw_context& dc, vec2 p0, vec2 p1, vec2 p2, vec2 p3, rgba c0, rgba c1, rgba c2, rgba c3, int tile_num, int flags);
void draw_tile(const draw_context& dc, vec2 c, float s, float rot, rgba col, int tile_num, int flags);
void draw_tile(const draw_context& dc, vec2 c, float s, rgba col, int tile_num, int flags);
void draw_tile(const draw_context& dc, vec2 p0, vec2 p1, rgba col, int tile_num, int flags);
void draw_tile(const draw_context& dc, vec2 p0, vec2 p1, rgba c0, rgba c1, rgba c2, rgba c3, int tile_num, int flags);

void draw_tex_tile(const draw_context& dc, vec2 p0, vec2 p1, vec2 p2, vec2 p3, rgba c0, rgba c1, rgba c2, rgba c3, int flags);
void draw_tex_tile(const draw_context& dc, vec2 c, float s, float rot, rgba col, int flags);
void draw_tex_tile(const draw_context& dc, vec2 c, float s, rgba col, int flags);
void draw_tex_tile(const draw_context& dc, vec2 p0, vec2 p1, rgba col, int flags);
void draw_tex_tile(const draw_context& dc, vec2 p0, vec2 p1, rgba c0, rgba c1, rgba c2, rgba c3, int flags);

void init_fullscreen_quad();
void draw_fullscreen_quad(pipeline pl);

// font

enum font_flags {
	TEXT_LEFT	= 1,
	TEXT_RIGHT	= 2,
	TEXT_CENTRE	= 3,
	TEXT_TOP	= 4,
	TEXT_BOTTOM	= 8,
	TEXT_VCENTRE= 12
};

void font_init();

vec2 measure_string(const char* s);
void draw_string(const draw_context& dc, vec2 pos, vec2 scale, int flags, rgba col, const char* txt);
void draw_stringf(const draw_context& dc, vec2 pos, vec2 scale, int flags, rgba col, const char* fmt, ...);

// bloom

struct bloom_level {
	vec2 size;
	texture reduce;
	texture blur_x;
	texture blur_y;
};

struct bloom_fx {
	static const int MAX_LEVELS = 5;
	bloom_level levels[MAX_LEVELS];

	void init_resources(vec2i view_size);
	void render_levels_from_source(texture source);
	void assign_levels_to_texture_group(texture_group* tg, int first_slot);
};

// debug_draw

void debug_log(rgba c, const char* fmt, ...);
void debug_text(vec2 p, rgba c, const char* fmt, ...);

void debug_line(vec2 a, vec2 b, rgba colour, int duration_in_frames);
void debug_circle(vec2 p, float radius, rgba colour, int duration_in_frames);
void debug_rect(vec2 a, vec2 b, rgba colour, int duration_in_frames);
void debug_filled_rect(vec2 a, vec2 b, rgba colour, int duration_in_frames);

void debug_set_proj_view(const mat44& proj_view);

void debug_init();
void debug_step();
void debug_render(vec2 view_size);

// assets

struct font {
	texture texture;
	u8 glyph_width[128];
};

struct find_asset_data {
	int file;
	u32 bank;
	u32 entry;

	find_asset_data() : file(), bank(), entry() { }
};

bool load_assets(const char* path);
void* find_asset(u32 code, const char* name);
void* find_next_asset(find_asset_data* fad, u32 code, const char** name);

texture load_texture(const char* name);
shader load_shader(input_layout* il, const char* name);
font load_font(const char* name);

// sound

enum class sfx {
	DIT,
	PLANET_DIE,
	PLANET_HURT,
	PLANET_GROW,
	PLANET_HEAL,
	PLAYER_FIRE,
	PLAYER_FIRE_ALT,
	PLAYER_TETHER_READY,
	PLAYER_BUILD_READY,
	PLAYER_BUILD_TURRET,
	PLAYER_BUILD_GENERATOR,
	TRACKER_DIE,
	_MAX
};

void sound_init();
void sound_step();

void define_sound(sfx s, const char* name, int max_voices, int max_cooldown);

voice_id sound_play(sfx s, float semitones = 0.0f, float decibels = 0.0f, int flags = 0);
void sound_stop(sfx s);