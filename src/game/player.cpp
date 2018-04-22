#include "pch.h"
#include "game.h"

player::player() : entity(ET_PLAYER) {
	_flags |= EF_PLAYER | EF_UNIT;
	_radius = 5.0f;
	_colour = rgba(1.0f);//0.75f, 1.0f, 0.5f, 1.0f);
	_time_since_hit = 0.0f;
}

void player::init() {
}

void player::tick() {
	vec2 key_move = g_input.pad_left;

	if (is_key_down(g_input.bind_left )) key_move.x -= 1.0f;
	if (is_key_down(g_input.bind_right)) key_move.x += 1.0f;
	if (is_key_down(g_input.bind_up   )) key_move.y -= 1.0f;
	if (is_key_down(g_input.bind_down )) key_move.y += 1.0f;

	if (is_key_down(KEY_LEFT )) key_move.x -= 1.0f;
	if (is_key_down(KEY_RIGHT)) key_move.x += 1.0f;
	if (is_key_down(KEY_UP   )) key_move.y -= 1.0f;
	if (is_key_down(KEY_DOWN )) key_move.y += 1.0f;

	if (length_sq(key_move) > 1.0f)
		key_move = normalise(key_move);

	if (is_key_pressed(KEY_1)) buy(ET_TURRET);
	if (is_key_pressed(KEY_2)) buy(ET_COLLECTOR);
	if (is_key_pressed(KEY_3)) buy(ET_INCITER);

	//if (is_key_pressed(KEY_F5)) {
	//	g_world.resources += 150;
	//}

	key_move += to_vec2(g_input.mouse_rel) * 0.05f;

	//if (is_key_down(KEY_SPACE) || g_input.mouse_buttons)
	//	key_move = 0.0f;

	_vel += key_move * 128.0f;

	entity_move_slide(this);

	avoid_buildings(&g_world, this);

	if (_time_since_hit > 0.0f) {
		_time_since_hit -= DT;

		if (_time_since_hit > 0.5f) {
			int c = clamp((int)(3.0f * (_time_since_hit - 0.5f)), 1, 3);
			fx_explosion(_pos, 0.5f, c, rgba(1.0f, 0.1f, 0.1f, 1.0f), 0.5f);
		}
	}
	else
		_time_since_hit = 0.0f;

	_vel *= 0.5f;
}

void player::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape(vec2(), 8, 6.0f, PI * 0.25f, rgba(0.0f, 1.0f));
	dc->shape(vec2(), 8, 2.0f, PI * 0.25f, _colour);
	dc->shape_outline(vec2(), 8, 5.0f, PI * 0.25f, 0.5f, _colour);
}

void player::hit() {
	if (_time_since_hit > 0.5f) {
		if ((PLAYER_VULNERABLE_TIME - _time_since_hit) > 0.25f) {
			sound_play(sfx::PLAYER_DIE);
			fx_explosion(_pos, 1.0f, 40, rgba(), 1.0f);
			destroy_entity(this);
		}
	}
	else {
		g_world.resources = max((g_world.resources / 2) - 5, 0);
		sound_play(sfx::PLAYER_HIT);
		fx_explosion(_pos, 1.0f, 30, rgba(1.0f, 0.1f, 0.1f, 1.0f), 1.0f);
		_time_since_hit = PLAYER_VULNERABLE_TIME;

		for_all([&](entity* e) {
			if (e->_type == ET_DRONE) {
				vec2 d = e->_pos - _pos;

				if (length_sq(d) < square(120.0f)) {
					if (length_sq(d) > 1.0f) {
						e->_vel = normalise(d) * 200.0f;
						((drone*)e)->_aggressive = 1.0f;
					}
				}
			}
		});
	}
}

void player::buy(entity_type item) {
	int cost = 0;

	switch(item) {
		case ET_TURRET:    cost = TURRET_COST;    break;
		case ET_COLLECTOR: cost = COLLECTOR_COST; break;
		case ET_INCITER:   cost = INCITER_COST;   break;
	}

	if ((cost <= 0) || (cost > g_world.resources)) {
		switch(item) {
			case ET_TURRET:    g_world.error_hotbar_turret    = 0.5f; break;
			case ET_COLLECTOR: g_world.error_hotbar_collector = 0.5f; break;
			case ET_INCITER:   g_world.error_hotbar_inciter   = 0.5f; break;
		}

		sound_play(sfx::UI_NO);

		fx_message("not enough $$$");

		return;
	}

	vec2 shop_ofs;

	do {
		shop_ofs = g_world.r.range(_radius);
	} while(length_sq(shop_ofs) < (_radius * 0.5f));

	if (entity* e = find_building_near_point(_pos, 32.0f)) {
		fx_circle(e->_pos, e->_radius + 28.0f, 64, 1.0f, rgba(0.5f, 0.0f, 0.0f, 1.0f));
		fx_circle(e->_pos, e->_radius + 30.0f, 64, 1.0f, rgba(1.0f, 0.0f, 0.0f, 1.0f));

		sound_play(sfx::UI_NO);

		fx_message("too close to existing structure");

		return;
	}

	sound_play(sfx::PLAYER_BUILD, 3.0f);

	g_world.resources -= cost;

	switch(item) {
		case ET_TURRET:		spawn_entity(new turret, _pos + shop_ofs);		break;
		case ET_COLLECTOR:	spawn_entity(new collector, _pos + shop_ofs);	break;
		case ET_INCITER:	spawn_entity(new inciter, _pos + shop_ofs);		break;
	}

	switch(item) {
		case ET_TURRET:    g_world.flash_hotbar_turret    = 0.25f; break;
		case ET_COLLECTOR: g_world.flash_hotbar_collector = 0.25f; break;
		case ET_INCITER:   g_world.flash_hotbar_inciter   = 0.25f; break;
	}
}
