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

extern random g_rand;

const wchar_t* g_win_name = L"LD41 - ???";

draw_list g_dl_world;
draw_list g_dl_ui;

#define USE_LOOPS 0

voice_id g_turret_loop;
voice_id g_collector_loop;
voice_id g_inciter_loop;

void game_init() {
	g_dl_world.init(256 * 1024, 4096);
	g_dl_ui.init(256 * 1024, 4096);

	define_sound(sfx::DIT,            "dit",            2, 2);
	define_sound(sfx::UI_NO,          "ui_no",          2, 2);
	define_sound(sfx::PLAYER_DIE,     "player_die",     2, 2);
	define_sound(sfx::PLAYER_HIT,     "player_hit",     2, 2);
	define_sound(sfx::PLAYER_BUILD,   "player_build",   2, 2);
	define_sound(sfx::DRONE_SPOT,     "drone_spot",     2, 4);
	define_sound(sfx::DRONE_HIT,      "drone_hit",      2, 2);
	define_sound(sfx::DRONE_POP,      "drone_pop",      2, 2);
	define_sound(sfx::TURRET_FIRE,    "turret_fire",    4, 2);
	define_sound(sfx::TURRET_LOOP,    "turret_loop",    3, 0);
	define_sound(sfx::TURRET_ACQ,     "turret_acq",     2, 2);
	define_sound(sfx::COLLECTOR_ACQ,  "collector_acq",  2, 2);
	define_sound(sfx::INCITER_ACQ,    "inciter_acq",    2, 2);
	define_sound(sfx::PICKUP,         "pickup",         10, 2);

	psys_init(8000);

#if USE_LOOPS
	g_turret_loop = sound_play(sfx::TURRET_LOOP, -5, -1000.0f, SOUND_LOOP);
	g_collector_loop = sound_play(sfx::TURRET_LOOP, 0, -1000.0f, SOUND_LOOP);
	g_inciter_loop = sound_play(sfx::TURRET_LOOP, -10, -1000.0f, SOUND_LOOP);
#endif
}

void start_game() {
	world* w = &g_world;

	w->entities.free();
	w->handles.free();
	w->salt = 0;

	w->camera_pos    = vec2();
	w->camera_vel    = vec2();
	w->camera_target = vec2();

	w->limit = aabb2(-1000.0f, 1000.0f);

	w->resources = 0;
	w->total_resources = 0;

	w->flash_hotbar_turret = 0;
	w->flash_hotbar_collector = 0;
	w->flash_hotbar_inciter = 0;

	w->error_hotbar_turret = 0;
	w->error_hotbar_collector = 0;
	w->error_hotbar_inciter = 0;

	w->message = 0;
	w->message_time = 0;

	spawn_entity(new player, vec2());
	spawn_entity(new turret, vec2(0.0f, -30.0f));

	for(int y = -6; y < 6; y++) {
		for(int x = -6; x <= 6; x++) {
			if (abs(x) + abs(y) >= 2) {
				vec2 p = vec2((f32)x, (f32)y) * 150.0f + w->r.range(vec2(-55.0f, 55.0f));
				spawn_entity(new hive, p);
			}
		}
	}
}

enum game_state {
	STATE_MENU,
	STATE_GAME,
	STATE_SHOP,
	STATE_PAUSE,
	STATE_RESULTS
};

game_state g_state;

void game_frame(vec2 view_size) {
	world* w = &g_world;

	g_dl_world.reset();
	g_dl_ui.reset();

	g_request_mouse_capture = (g_state == STATE_GAME);

	draw_context dc(g_dl_world);
	draw_context dc_ui(g_dl_ui);

	w->view_size = view_size;

	aabb2 ui_rect;
	mat44 ui_vp = fit_ui_proj_view(640.0f, 360.0f, w->view_size.x / w->view_size.y, -64.0f, 64.0f, &ui_rect);

	// update

	auto draw_bar = [&](float y0, float y1, rgba c) {
			float x0 = ui_rect.min.x - 5.0f;
			float x1 = ui_rect.max.x + 5.0f;
			dc_ui.rect({ x0, y0 }, { x1, y1 }, rgba(0.0f, 0.95f));
			dc_ui.rect_outline({ x0, y0 }, { x1, y1 }, 1.0f, c);
		};

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
			sound_play(sfx::DIT);
			start_game();
		}
	}
	else if (g_state == STATE_GAME) {
		{
			char buf[256];
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%i", w->resources);

			vec2 size = measure_string(buf);

			size.x = max(size.x, 30.0f) + 10.0f;

			auto u = dc_ui.copy().translate(ui_rect.max.x - ((ui_rect.max.x - ui_rect.min.x) - (10.0f + 5.0f + size.x + 5.0f) * 1.25f), ui_rect.max.y - 4.0f).scale(1.25f);

			u.rect(vec2(-5.0f - size.x - 5.0f, -5.0f - size.y - 5.0f), vec2(-5.0f, -5.0f), rgba(0.0f, 1.0f));
			u.rect_outline(vec2(-5.0f - size.x - 5.0f, -5.0f - size.y - 5.0f), vec2(-5.0f, -5.0f), 0.5f, rgba(0.75f, 1.0f, 0.0f, 1.0f));

			auto draw_dollar = [&](float ox, float oy, rgba c) {
					draw_string(u, vec2(-size.x - 7.5f + ox, -7.5f - size.y * 0.5f + oy), vec2(2.0f), TEXT_CENTRE | TEXT_VCENTRE, c, "$");
				};

			draw_dollar(-1.0f, 0.0f, rgba(0.0f, 1.0f));
			draw_dollar(+1.0f, 0.0f, rgba(0.0f, 1.0f));
			draw_dollar(0.0f, -1.0f, rgba(0.0f, 1.0f));
			draw_dollar(0.0f, +1.0f, rgba(0.0f, 1.0f));
			draw_dollar(0.0f, 0.0f, rgba(0.75f, 1.0f, 0.0f, 1.0f));

			draw_string(u, vec2(-7.5f, -7.5f), vec2(1.0f), TEXT_RIGHT | TEXT_BOTTOM, rgba(0.75f, 1.0f, 0.0f, 1.0f), buf);
		}

		auto draw_item = [&](vec2 p, int cost, int key, const char* label, rgba colour, float& flash, float& error) {
				auto& u = dc_ui.copy().translate(p);

				if (flash > 0.0f) flash -= DT; else flash = 0.0f;
				if (error > 0.0f) error -= DT; else error = 0.0f;

				rgba b = (g_world.resources >= cost) ? rgba() : rgba(0.5f);
				rgba c = (g_world.resources >= cost) ? rgba(0.75f, 1.0f, 0.0f, 1.0f) : rgba(0.4f, 0.2f, 0.0f, 1.0f);
				rgba d = (g_world.resources >= cost) ? colour : rgba(0.25f, 1.0f);

				b = lerp(b, rgba(1.5f, 1.0f), flash * 4.0f);
				c = lerp(c, rgba(1.5f, 1.0f), flash * 4.0f);
				d = lerp(d, rgba(1.5f, 1.0f), flash * 4.0f);

				c = lerp(c, rgba(1.5f, 0.85f, 0.85f, 1.0f), fmodf(error * 4.0f, 1.0f));

				u.shape(vec2(20.0f, 20.0f), 32, 16.0f, 0.0f, rgba(0.0f, 1.0f));
				u.shape_outline(vec2(20.0f, 20.0f), 32, 16.0f, 0.0f, 0.5f, d * 0.75f);

				u.shape_outline(vec2(20.0f, 20.0f), 4, 8.0f, 0.0f, 0.5f, d);
				u.shape_outline(vec2(20.0f, 20.0f), 4, 8.0f, PI * 0.25f, 0.5f, d);
				draw_string(u, vec2(20.0f, 20.0f), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, d, label);

				u.rect(vec2(14.0f, 30.0f), vec2(26.0f, 42.0f), rgba(0.0f, 1.0f));
				u.rect_outline(vec2(14.0f, 30.0f), vec2(26.0f, 42.0f), 0.5f, b * 0.75f);
				draw_stringf(u, vec2(20.0f, 36.0f), vec2(0.5f), TEXT_CENTRE | TEXT_VCENTRE, b, "%i", key);
				
				u.rect(vec2(10.0f, 0.0f), vec2(30.0f, 6.0f), rgba(0.0f, 1.0f));
				u.rect_outline(vec2(10.0f, 0.0f), vec2(30.0f, 6.0f), 0.5f, c * 0.75f);
				draw_stringf(u, vec2(20.0f, 3.0f), vec2(0.5f), TEXT_CENTRE | TEXT_VCENTRE, c, "$ %i", cost);
			};

		draw_item(vec2(ui_rect.max.x - 125, ui_rect.max.y - 50.0f), TURRET_COST, 1, "T", rgba(0.1f, 0.85f, 0.25f, 1.0f), w->flash_hotbar_turret, w->error_hotbar_turret);
		draw_item(vec2(ui_rect.max.x - 85, ui_rect.max.y - 50.0f), COLLECTOR_COST, 2, "C", rgba(0.5f, 0.1f, 1.0f, 1.0f), w->flash_hotbar_collector, w->error_hotbar_collector);
		draw_item(vec2(ui_rect.max.x - 45, ui_rect.max.y - 50.0f), INCITER_COST, 3, "X", rgba(0.25f, 0.75f, 0.75f, 1.0f), w->flash_hotbar_inciter, w->error_hotbar_inciter);

		if (w->message) {
			w->message_time -= DT;

			if (w->message_time > 0.0f) {
				float f0 = clamp(w->message_time * 3.0f, 0.0f, 1.0f);
				float f1 = 1.0f - clamp((1.5f - w->message_time) * 3.0f, 0.0f, 1.0f);
				float f = lerp(f0, 1.5f, f1);

				draw_string(dc_ui, vec2(320.0f, 280.0f), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(f), w->message);
			}
			else {
				w->message = 0;
				w->message_time = 0;
			}
		}

		//if (g_input.mouse_buttons_pressed || is_key_pressed(KEY_SPACE)) {
		//	if (get_player())
		//		g_state = STATE_SHOP;
		//}

		if (is_key_pressed(KEY_ESCAPE)) {
			g_state = STATE_PAUSE;
			sound_play(sfx::DIT);
		}

		if (!get_player()) {
			g_state = STATE_RESULTS;
		}
	}
	else if (g_state == STATE_SHOP) {
		if ((!g_input.mouse_buttons && !is_key_down(KEY_SPACE)) || !get_player())
			g_state = STATE_GAME;

		dc_ui.rect(ui_rect.min, ui_rect.max, rgba(0.0f, 0.1f, 0.0f, 0.25f));

		for(int i = 0; i < 100; i++) {
			float y = lerp(ui_rect.min.y, ui_rect.max.y, i / 100.0f);
			dc_ui.line({ ui_rect.min.x, y }, { ui_rect.max.x, y }, 0.5f, rgba(0.0f, 0.0f, 0.0f, 0.25f));
		}
	}
	else if (g_state == STATE_PAUSE) {
		draw_bar(60.0f, 100.0f, rgba(1.0f));
		draw_bar(270.0f, 310.0f, rgba(1.0f));

		draw_string(dc_ui, vec2(320.0f, 80.0f), vec2(2.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "PAUSED");

		draw_string(dc_ui, vec2(320.0f, 280.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.95f, 0.95f, 0.0f, 1.0f), "press Q to quit");
		draw_string(dc_ui, vec2(320.0f, 300.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "escape or click to resume");

		if (is_key_pressed(KEY_Q)) {
			g_state = STATE_MENU;

			for(auto& e : w->entities)
				destroy_entity(e);
		}

		if (g_input.mouse_buttons || is_key_pressed(KEY_SPACE) || is_key_pressed(KEY_ESCAPE)) {
			g_state = STATE_GAME;
			sound_play(sfx::DIT);
		}
	}
	else if (g_state == STATE_RESULTS) {
		draw_bar(60.0f, 140.0f, rgba(1.0f));
		draw_bar(290.0f, 310.0f, rgba(1.0f));

		draw_string(dc_ui, vec2(320.0f, 80.0f), vec2(3.0f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "oh noes");

		draw_stringf(dc_ui, vec2(320.0f, 120.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.75f, 1.0f, 0.0f, 1.0f), "total collected: $ %i", w->total_resources);

		draw_string(dc_ui, vec2(320.0f, 300.0f), vec2(1.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(), "click to continue");

		if (g_input.start) {
			g_state = STATE_MENU;

			for(auto& e : w->entities)
				destroy_entity(e);
		}
	}

	world_tick(g_state != STATE_GAME);

	if ((g_state != STATE_PAUSE) && (g_state != STATE_SHOP))
		psys_update();

	// camera

	if (player* pl = get_player()) {
		if (g_state != STATE_PAUSE) {
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
	}

	// view

	w->view_proj = top_down_proj_view(-w->camera_pos, 90.0f, w->view_size.x / w->view_size.y, 400.0f, 1.0f, 1000.0f);

	// draw

	world_draw(&dc);
	
	//if (g_state == STATE_SHOP) {
	//if (g_input.mouse_buttons || is_key_down(KEY_SPACE)) {
	//	if (player* pl = get_player()) {
	//		//dc.shape_outline(pl->_pos, 64, 16.0f, 0.0f, 0.5f, rgba());
	//
	//		for(int i = 0; i < 3; i++) {
	//			float f = (i / 3.0f) * TAU;
	//
	//			dc.shape(pl->_pos + 48.0f * rotation(f), 64, 24.0f, 0.0f, rgba(0.0f, 0.9f));
	//			dc.shape_outline(pl->_pos + 48.0f * rotation(f), 64, 24.0f, 0.0f, 0.5f, rgba());
	//		}
	//	}
	//}

	if (player* pl = get_player()) {
		random r(0);

		float f = clamp(pl->_time_since_hit, 0.0f, 1.0f);
		float t = pl->_time_since_hit;
		
		if (f > 0.0f) {
			float x0 = ui_rect.min.x;
			float x1 = ui_rect.max.x;
			float y0 = ui_rect.min.y;
			float y1 = ui_rect.max.y;
			float xc = (x0 + x1) * 0.5f;
			float yc = (y0 + y1) * 0.5f;

			rgba c = rgba(0.25f, 0.0f, 0.0f, 0.0f) * f;
			rgba d = rgba(0.0f, 0.0f, 0.0f, 0.0f);

			dc_ui.rect({ x0, y0 }, { xc, yc }, c, c, c, d);
			dc_ui.rect({ xc, y0 }, { x1, yc }, c, c, d, c);
			dc_ui.rect({ x0, yc }, { xc, y1 }, c, d, c, c);
			dc_ui.rect({ xc, yc }, { x1, y1 }, d, c, c, c);

			for(int i = 0; i < 100; i++) {
				float x;
				float y;

				if (r.chance(1, 2)) {
					x = r.chance(1, 2) ? x0 : x1;
					y = r.range(y0, y1);
				}
				else {
					x = r.range(x0, x1);
					y = r.chance(1, 2) ? y0 : y1;
				}

				float s = 10.0f + square(i / 100.0f) * 150.0f;
				dc_ui.shape({ x, y }, 4, s + r.range(20.0f), r.range(PI) - t * r.range(PI * 4.0f), c * 0.5f);
			}
		}
	}

#if USE_LOOPS
	// loops

	float turret_volume = -1000.0f;
	float collector_volume = -1000.0f;
	float inciter_volume = -1000.0f;

	if (player* pl = get_player()) {
		if (entity* e = find_entity_near_point(pl->_pos, 250.0f, ET_TURRET)) {
			turret_volume = -(length(pl->_pos - e->_pos) / 250.0f) * 60.0f - 9.0f;
		}

		if (entity* e = find_entity_near_point(pl->_pos, 250.0f, ET_COLLECTOR)) {
			collector_volume = -(length(pl->_pos - e->_pos) / 250.0f) * 60.0f - 10.0f;
		}

		if (entity* e = find_entity_near_point(pl->_pos, 250.0f, ET_INCITER)) {
			inciter_volume = -(length(pl->_pos - e->_pos) / 250.0f) * 60.0f - 7.0f;
		}
	}

	audio_set_volume(g_turret_loop, turret_volume);
	audio_set_volume(g_collector_loop, collector_volume);
	audio_set_volume(g_inciter_loop, inciter_volume);
#endif
	// render

	gpu_set_pipeline(g_pipeline_sprite);
	gpu_set_const(0, w->view_proj);
	g_dl_world.render();

	gpu_set_const(0, ui_vp);
	g_dl_ui.render();

	debug_set_proj_view(w->view_proj);
}