#include "pch.h"
#include "base.h"
#include "sys.h"

pipeline g_pipeline_sprite;
pipeline g_pipeline_bloom_reduce;
pipeline g_pipeline_bloom_blur_x;
pipeline g_pipeline_bloom_blur_y;
pipeline g_pipeline_combine;

void init_render_state() {
	input_layout il_xyz;
	il_xyz.element(offsetof(v_xyz_uv_rgba, x), gpu_type::F32_3, gpu_usage::POSITION, 0);
	il_xyz.element(offsetof(v_xyz_uv_rgba, u), gpu_type::F32_2, gpu_usage::UV, 0);
	il_xyz.element(offsetof(v_xyz_uv_rgba, r), gpu_type::F32_4, gpu_usage::COLOUR, 0);

	input_layout il_xyzw;
	il_xyzw.element(offsetof(v_xyzw_uv, x), gpu_type::F32_4, gpu_usage::POSITION, 0);
	il_xyzw.element(offsetof(v_xyzw_uv, u), gpu_type::F32_2, gpu_usage::UV, 0);

	shader       shader_blur_x        = load_shader(&il_xyzw, "fs_blur_x");
	shader       shader_blur_y        = load_shader(&il_xyzw, "fs_blur_y");
	shader       shader_combine       = load_shader(&il_xyzw, "fs_combine");;
	shader       shader_reduce        = load_shader(&il_xyzw, "fs_reduce");
	shader       shader_sprite        = load_shader(&il_xyz, "uv_c");

	blend_state  blend_default        = gpu_create_blend_state(false, gpu_blend::ONE, gpu_blend::ZERO, gpu_blend_op::ADD, gpu_blend::ONE, gpu_blend::ZERO, gpu_blend_op::ADD);
	blend_state  blend_premul_alpha   = gpu_create_blend_state(true, gpu_blend::ONE, gpu_blend::INV_SRC_ALPHA, gpu_blend_op::ADD, gpu_blend::ONE, gpu_blend::INV_SRC_ALPHA, gpu_blend_op::ADD);

	depth_state  depth_default        = gpu_create_depth_state(false, false);

	raster_state raster_default       = gpu_create_raster_state(gpu_cull::NONE, true);

	sampler      sampler_point_clamp  = gpu_create_sampler(gpu_filter::MIN_MAG_MIP_POINT, gpu_address::CLAMP, gpu_address::CLAMP, gpu_address::CLAMP);
	sampler      sampler_linear_clamp = gpu_create_sampler(gpu_filter::MIN_MAG_MIP_LINEAR, gpu_address::CLAMP, gpu_address::CLAMP, gpu_address::CLAMP);

	g_pipeline_sprite       = gpu_create_pipeline(shader_sprite,  blend_premul_alpha, depth_default, raster_default, { sampler_point_clamp },                       gpu_primitive::TRIANGLE_LIST);
	g_pipeline_bloom_reduce = gpu_create_pipeline(shader_reduce,  blend_default,      depth_default, raster_default, { sampler_linear_clamp },                      gpu_primitive::TRIANGLE_LIST);
	g_pipeline_bloom_blur_x = gpu_create_pipeline(shader_blur_x,  blend_default,      depth_default, raster_default, { sampler_linear_clamp },                      gpu_primitive::TRIANGLE_LIST);
	g_pipeline_bloom_blur_y = gpu_create_pipeline(shader_blur_y,  blend_default,      depth_default, raster_default, { sampler_linear_clamp },                      gpu_primitive::TRIANGLE_LIST);
	g_pipeline_combine      = gpu_create_pipeline(shader_combine, blend_default,      depth_default, raster_default, { sampler_point_clamp, sampler_linear_clamp }, gpu_primitive::TRIANGLE_LIST);
}