#include "pch.h"
#include "game.h"

player::player() : entity(ET_PLAYER) {
	_flags |= EF_PLAYER | EF_UNIT;
	_radius = 5.0f;
	_colour = rgba(0.75f, 1.0f, 0.5f, 1.0f);
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

	if (is_key_pressed(KEY_1)) {
		if (g_world.resources >= 10) {
			spawn_entity(new turret, _pos);
			g_world.resources -= 10;
		}
	}

	if (is_key_pressed(KEY_2)) {
		if (g_world.resources >= 25) {
			spawn_entity(new collector, _pos);
			g_world.resources -= 25;
		}
	}

	if (is_key_pressed(KEY_3)) {
		if (g_world.resources >= 100) {
			spawn_entity(new inciter, _pos);
			g_world.resources -= 100;
		}
	}

	if (is_key_pressed(KEY_F5)) {
		g_world.resources += 150;
	}

	key_move += to_vec2(g_input.mouse_rel) * 0.05f;

	_vel += key_move * 128.0f;

	entity_move_slide(this);

	avoid_buildings(&g_world, this);

	_vel *= 0.5f;
}

void player::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape(vec2(), 8, 2.0f, PI * 0.25f, _colour);
	dc->shape_outline(vec2(), 8, 5.0f, PI * 0.25f, 0.5f, _colour);
}
