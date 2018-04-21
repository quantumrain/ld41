#include "pch.h"
#include "game.h"

/*

	* idle automation + bullet hell *

	hive
	- produces 1 enemy to kill every 10 ticks
	- commonly produces passive drones, but occasionally spawns aggressive enemies

	turret
	- kills 1 enemy every 20 ticks
	- can't be placed too close to hives
	- you have to collect the resources

	collector
	- collects resources in a radius
	- collects 1 resouce every 30 ticks

	generator
	- uses 5 resources a tick, but doubles the speed of everything in radius
	- overlapping power increases production linearly (e.g. x3 instead of x2)

	inciter
	- hives in radius produce two enemies every 10 ticks
	- overlapping inciters increase production linearly
	- twice as likely to spawn aggressive enemy

*/

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

	w->camera_pos    = vec2();
	w->camera_vel    = vec2();
	w->camera_target = vec2();

	spawn_entity(new player, vec2());
	spawn_entity(new turret, vec2(-50.0f, 0.0f));
	spawn_entity(new turret, vec2(+50.0f, 0.0f));

	for(int y = -10; y < 10; y++) {
		for(int x = -10; x < 10; x++) {
			if (x && y)
			{
				vec2 p = vec2(x, y) * 150.0f + w->r.range(vec2(-50.0f, 50.0f));
				spawn_entity(new hive, p);
			}
		}
	}
}

enum game_state {
	STATE_MENU,
	STATE_GAME,
	STATE_PAUSE,
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
			g_state = STATE_PAUSE;
		}

		if (!get_player()) {
			g_state = STATE_RESULTS;
		}
	}
	else if (g_state == STATE_PAUSE) {
		draw_string(dc_ui, vec2(320.0f, 80.0f), vec2(2.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "PAUSED");

		draw_string(dc_ui, vec2(320.0f, 300.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.6f), "press escape to resume");

		if (is_key_pressed(KEY_ESCAPE)) {
			g_state = STATE_GAME;

			//for(auto& e : w->entities)
			//	destroy_entity(e);
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

	world_tick(g_state != STATE_GAME);

	psys_update();

	// camera

	if (player* pl = get_player()) {
		vec2 player_pos = pl->_pos;

		vec2  target_pos   = player_pos;
		vec2  target_delta = target_pos - w->camera_target;

		w->camera_target = target_pos;

		vec2 camera_delta = w->camera_target - w->camera_pos;

		vec2 lim = vec2 { 320.0f, 180.0f } * 0.9f;

		if (fabsf(camera_delta.x) > lim.x) pl->_pos.x = w->camera_pos.x + lim.x * isign(camera_delta.x);
		if (fabsf(camera_delta.y) > lim.y) pl->_pos.y = w->camera_pos.y + lim.y * isign(camera_delta.y);

		//camera_delta.x = max(fabsf(camera_delta.x) - 80.0f, 0.0f) * isign(camera_delta.x);
		//camera_delta.y = max(fabsf(camera_delta.y) - 45.0f, 0.0f) * isign(camera_delta.y);

		w->camera_pos += camera_delta * 0.05f;
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