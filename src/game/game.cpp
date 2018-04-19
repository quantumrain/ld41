#include "pch.h"
#include "game.h"

const wchar_t* g_win_name = L"LD41 - ???";

draw_list g_dl_world;
draw_list g_dl_ui;

void game_init() {
	g_dl_world.init(256 * 1024, 4096);
	g_dl_ui.init(256 * 1024, 4096);

	define_sound(sfx::DIT, "dit", 2, 2);

	psys_init(1000);
}

void start_game() {
	world* w = &g_world;

	w->entities.free();
	w->handles.free();

	spawn_entity(new player, vec2());
}

enum game_state {
	STATE_MENU,
	STATE_GAME,
	STATE_RESULTS
};

game_state g_state;

void game_frame(vec2 view_size) {
	world* w = &g_world;

	g_dl_world.reset();
	g_dl_ui.reset();

	g_request_mouse_capture = g_state == STATE_GAME;

	draw_context dc(g_dl_world);
	draw_context dc_ui(g_dl_ui);

	w->view_size = view_size;

	// update

	if (g_state == STATE_MENU) {

		draw_string(dc_ui, vec2(320.0f, 80.0f), vec2(4.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "???");
		draw_string(dc_ui, vec2(320.0f, 110.0f), vec2(0.75f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.6f), "created for ludum dare 41 by stephen cakebread @quantumrain");

		draw_string(dc_ui, vec2(320.0f, 160.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.8f), "controls - ???");
		draw_string(dc_ui, vec2(320.0f, 175.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.8f), "fullscreen - F11");

		draw_string(dc_ui, vec2(320.0f, 220.0f), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(1.0f), "blah blah");
		draw_string(dc_ui, vec2(320.0f, 230.0f), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.6f), "blah blah");
		draw_string(dc_ui, vec2(320.0f, 240.0f), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.6f), "blah blah");

		draw_string(dc_ui, vec2(320.0f, 300.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "left click to start");

		if (g_input.start) {
			g_state = STATE_GAME;
			start_game();
		}
	}
	else if (g_state == STATE_GAME) {
		if (is_key_pressed(KEY_ESCAPE)) {
			g_state = STATE_MENU;

			for(auto& e : w->entities)
				destroy_entity(e);
		}

		if (!get_player()) {
			g_state = STATE_RESULTS;
		}
	}
	else if (g_state == STATE_RESULTS) {
		draw_string(dc_ui, vec2(320.0f, 80.0f), vec2(2.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "oh noes");

		if (g_input.start) {
			g_state = STATE_MENU;

			for(auto& e : w->entities)
				destroy_entity(e);
		}
	}

	world_tick(g_state == STATE_MENU);

	psys_update();

	// camera

	if (player* pl = get_player()) {
		vec2 player_pos = vec2();//pl->_pos;

		vec2  target_pos   = player_pos;
		vec2  target_delta = target_pos - w->camera_target;

		w->camera_old_target = target_pos;

		vec2 camera_delta = (w->camera_target - w->camera_pos) * 0.1f;

		w->camera_pos += camera_delta;
	}

	// view

	w->view_proj = top_down_proj_view(-w->camera_pos, 90.0f, w->view_size.x / w->view_size.y, 400.0f, 1.0f, 1000.0f);

	// draw

	world_draw(&dc);

	// render

	gpu_set_pipeline(g_pipeline_sprite);
	gpu_set_const(0, w->view_proj);
	g_dl_world.render();

	gpu_set_const(0, fit_ui_proj_view(640.0f, 360.0f, w->view_size.x / w->view_size.y, -64.0f, 64.0f));
	g_dl_ui.render();

	debug_set_proj_view(w->view_proj);
}