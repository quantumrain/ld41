#include "pch.h"
#include "game.h"

turret::turret() : entity(ET_TURRET) {
	_flags |= EF_BUILDING;
	_radius = 10.0f;
	_colour = rgba(0.1f, 0.85f, 0.25f, 1.0f);
	_time = 0.0f;
}

void turret::init() {
}

void turret::tick() {
	_time -= DT;

	if (_time < 0.0f) {
		if (entity* e = find_enemy_near_point(_pos, 150.0f)) {
			bullet* b = spawn_entity(new bullet, _pos);

			sound_play(sfx::DIT, -20.0f, -6.0f, 0);

			b->_vel = normalise(e->_pos - _pos) * b->_speed;
			b->_target = get_entity_handle(e);

			_time = 1.0f;
		}
	}

	_vel *= 0.5f;
}

void turret::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, _colour);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, _colour);

	if (_lifetime < 3.0f) {
		dc->shape_outline(vec2(), 64, lerp(0.0f, 150.0f, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.25f, _colour * lerp(0.25f, 0.0f, clamp((_lifetime - 1.0f) / 2.0f, 0.0f, 1.0f)));
	}

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, _colour, "T");
}
