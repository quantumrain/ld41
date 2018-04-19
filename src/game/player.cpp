#include "pch.h"
#include "game.h"

player::player() : entity(ET_PLAYER) {
	_flags |= EF_PLAYER;
	_radius = 5.0f;
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

	key_move += to_vec2(g_input.mouse_rel) * 0.1f;

	_vel += key_move * 128.0f;

	entity_move_slide(this);

	_vel *= 0.5f;
}

void player::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 32, _radius, 0.0f, 0.5f, rgba());
}
