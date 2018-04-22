#include "pch.h"
#include "game.h"

turret::turret() : entity(ET_TURRET) {
	_flags |= EF_BUILDING;
	_radius = 10.0f;
	_colour = rgba(0.2f, 1.15f, 0.35f, 1.0f);
	_time = 0.0f;
	_fired = 0.0f;
}

void turret::init() {
	fx_circle(_pos, TURRET_RANGE, 256, 1.0f, _colour * rgba(0.5f, 1.0f), 4);
	fx_circle(_pos, TURRET_RANGE, 256, 1.0f, _colour, 4);

	g_world.num_turrets++;
}

void turret::tick() {
	if (_time > 0.0f) {
		_time -= DT;

		if (g_world.ultimate) {
			_time -= DT * 4.0f;
		}
	}

	if (_time <= 0.0f) {
		if (entity* e = find_enemy_near_point(_pos, TURRET_RANGE)) {
			bullet* b = spawn_entity(new bullet, _pos);

			sound_play(sfx::TURRET_FIRE, -20.0f, -6.0f + get_vol(_pos), 0);

			b->_vel = normalise(e->_pos - _pos) * b->_speed;
			b->_target = get_entity_handle(e);

			_fired = 0.25f;
			_time += 2.0f;
		}
	}

	if (_fired > 0.0f)
		_fired -= DT;
	else
		_fired = 0.0f;

	_vel *= 0.5f;
}

void turret::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	rgba c = _colour;

	if (_fired > 0.0f)
		c = lerp(c, rgba(1.5f, 1.0f), _fired * 4.0f);

	//if (_lifetime < 1.0f)
	//	dc->shape_outline(vec2(), 64, lerp(0.0f, TURRET_RANGE, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.75f, _colour * lerp(0.75f, 0.0f, clamp((_lifetime - 0.5f) * 2.0f, 0.0f, 1.0f)));

	if (_fired > 0.0f)
		dc->scale(square(1.0f + _fired));

	dc->shape(vec2(), 4, _radius + 1.0f, 0.0f, c * rgba(0.2f, 1.0f));
	dc->shape(vec2(), 4, _radius + 1.0f, PI * 0.25f, c * rgba(0.2f, 1.0f));

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, c);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, c);

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, c, "T");
}
