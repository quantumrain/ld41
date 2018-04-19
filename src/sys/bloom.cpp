#include "pch.h"
#include "base.h"
#include "sys.h"

void bloom_fx::init_resources(vec2i view_size) {
	for(int i = 0; i < MAX_LEVELS; i++) {
		bloom_level* bl = levels + i;

		view_size /= 2;

		view_size.x = max(view_size.x, 1);
		view_size.y = max(view_size.y, 1);

		bl->size   = to_vec2(view_size);
		bl->reduce = gpu_discard_on_reset(gpu_create_texture(view_size.x, view_size.y, gpu_format::RGBA_16, gpu_bind::SHADER | gpu_bind::RENDER_TARGET, 0));
		bl->blur_x = gpu_discard_on_reset(gpu_create_texture(view_size.x, view_size.y, gpu_format::RGBA_16, gpu_bind::SHADER | gpu_bind::RENDER_TARGET, 0));
		bl->blur_y = gpu_discard_on_reset(gpu_create_texture(view_size.x, view_size.y, gpu_format::RGBA_16, gpu_bind::SHADER | gpu_bind::RENDER_TARGET, 0));
	}
}

void bloom_fx::render_levels_from_source(texture source) {
	for(int i = 0; i < MAX_LEVELS; i++) {
		bloom_level* bl = levels + i;

		gpu_set_const(0, vec4(1.0f / bl->size, 0.0f, 0.0f));
		gpu_set_viewport(vec2(), bl->size, vec2(0.0f, 1.0f));

		gpu_set_render_target(bl->reduce, 0);
		gpu_set_textures({ (i == 0) ? source : levels[i - 1].blur_y });
		draw_fullscreen_quad(g_pipeline_bloom_reduce);

		gpu_set_render_target(bl->blur_x, 0);
		gpu_set_textures({ bl->reduce });
		draw_fullscreen_quad(g_pipeline_bloom_blur_x);

		gpu_set_render_target(bl->blur_y, 0);
		gpu_set_textures({ bl->blur_x });
		draw_fullscreen_quad(g_pipeline_bloom_blur_y);
	}
}

void bloom_fx::assign_levels_to_texture_group(texture_group* tg, int first_slot) {
	for(int i = 0; i < MAX_LEVELS; i++)
		tg->texture[first_slot + i] = levels[i].blur_y;
}