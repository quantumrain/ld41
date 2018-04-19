#pragma once

#define GPU_TEXTURE_GROUP_SIZE 12
#define GPU_SAMPLER_GROUP_SIZE 4

enum class gpu_format {
	RGBA, RGBA_SRGB, RGBA_16, D24S8
};

namespace gpu_bind { enum {
	SHADER			= 0x1,
	RENDER_TARGET	= 0x2,
	DEPTH_STENCIL	= 0x4,
}; }

enum class gpu_primitive {
	LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP
};

enum class gpu_type {
	F32_1, F32_2, F32_3, F32_4,
	COLOUR,
	U8_4, U8_4_N,
	I16_2, I16_4,
	I16_2_N, I16_4_N,
	U16_2_N, U16_4_N
};

enum class gpu_blend {
	ZERO, ONE,
	SRC_COLOUR, INV_SRC_COLOUR, SRC_ALPHA, INV_SRC_ALPHA,
	DST_COLOUR, INV_DST_COLOUR, DST_ALPHA, INV_DST_ALPHA
};

enum class gpu_blend_op {
	ADD, SUB, REV_SUB, MIN, MAX
};

enum class gpu_address {
	WRAP, CLAMP, BORDER
};

enum class gpu_filter {
	MIN_MAG_MIP_POINT,
	MIN_MAG_POINT_MIP_LINEAR,
	MIN_POINT_MAG_LINEAR_MIP_POINT,
	MIN_POINT_MAG_MIP_LINEAR,
	MIN_LINEAR_MAG_MIP_POINT,
	MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MIN_MAG_LINEAR_MIP_POINT,
	MIN_MAG_MIP_LINEAR
};

enum class gpu_cull {
	NONE, FRONT, BACK
};

enum class gpu_usage {
	POSITION, NORMAL, UV, COLOUR
};

struct input_layout {
	enum { MAX_COUNT = 8 };

	int			count;
	u16			offset[MAX_COUNT];
	gpu_type	type[MAX_COUNT];
	gpu_usage	usage[MAX_COUNT];
	u8			usage_index[MAX_COUNT];

	input_layout() : count() { }

	void element(int offset_, gpu_type type_, gpu_usage usage_, int usage_index_);
};

#define GPU_HANDLE(name) struct name { u16 v; name(u16 v_ = 0) : v(v_) { } operator u16() { return v; } };

GPU_HANDLE(shader)
GPU_HANDLE(blend_state)
GPU_HANDLE(depth_state)
GPU_HANDLE(raster_state)
GPU_HANDLE(vertex_buffer)
GPU_HANDLE(texture)
GPU_HANDLE(sampler)

struct texture_group { texture texture[GPU_TEXTURE_GROUP_SIZE]; };
struct sampler_group { sampler sampler[GPU_SAMPLER_GROUP_SIZE]; };

struct pipeline {
	shader			shader;
	blend_state		blend;
	depth_state		depth;
	raster_state	raster;
	sampler_group	samplers;
	gpu_primitive	primitive;
};

void gpu_init(void* hwnd, vec2i view_size);
void gpu_shutdown();

void gpu_reset(vec2i view_size);

bool gpu_begin_frame();
void gpu_present();

void gpu_toggle_fullscreen();

shader			gpu_create_shader(input_layout* il, const u8* vs, int vs_size, const u8* ps, int ps_size);
blend_state		gpu_create_blend_state(bool alpha_blend_enable, gpu_blend src, gpu_blend dst, gpu_blend_op op, gpu_blend src_a, gpu_blend dst_a, gpu_blend_op op_a);
depth_state		gpu_create_depth_state(bool depth_enable, bool depth_write);
raster_state	gpu_create_raster_state(gpu_cull cull, bool fill_solid);
pipeline		gpu_create_pipeline(shader shader, blend_state blend, depth_state depth, raster_state raster, sampler_group	samplers, gpu_primitive primitive);
vertex_buffer	gpu_create_vertex_buffer(int vertex_stride, int num_vertices);
texture			gpu_create_texture(int width, int height, gpu_format fmt, u32 bind_flags, u8* initial_data);
sampler			gpu_create_sampler(gpu_filter filter, gpu_address address_u, gpu_address address_v, gpu_address address_w);

void gpu_destroy(shader si);
void gpu_destroy(blend_state bsi);
void gpu_destroy(depth_state dsi);
void gpu_destroy(raster_state rsi);
void gpu_destroy(vertex_buffer vbi);
void gpu_destroy(texture ti);
void gpu_destroy(sampler si);

texture gpu_discard_on_reset(texture ti);

void* gpu_map(vertex_buffer vb);
void gpu_unmap(vertex_buffer vb);

void gpu_set_render_target(texture rti, texture dsi);
texture gpu_backbuffer();

void gpu_set_viewport(vec2 pos, vec2 size, vec2 depth);
void gpu_set_pipeline(pipeline pl);
void gpu_set_vertex_buffer(vertex_buffer vbi);
void gpu_set_textures(texture_group tg);

void gpu_set_const(int slot, vec4 v);
void gpu_set_const(int slot, mat44 m);

void gpu_clear(texture ti, const rgba& c);
void gpu_draw(int count, int base);