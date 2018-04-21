#include "pch.h"
#include "game.h"

bullet::bullet() : entity(ET_BULLET) {
	_flags |= EF_BULLET;
	_radius = 5.0f;
	_colour = rgba(0.2f, 1.0f, 0.5f, 1.0f);
	_time = 2.0f;
	_speed = 50.0f;
}

void bullet::init() {
}

void bullet::tick() {
	_time -= DT;

	if (_time <= 0.0f)
		destroy_entity(this);

	for(int i = 0; i < 4; i++) {
		float f = i / 3.0f;
		psys_spawn(lerp(_old_pos, _pos, f) + g_world.r.range(vec2(1.0f)), vec2(), 3.0f, 1.0f, 1.0f, 0.5f, rgba(0.5f, 0.0f), 10);
	}

	_speed += DT * 400.0f;

	if (entity* e = get_entity(_target)) {
		vec2 d = e->_pos - _pos;
		_vel = normalise(_vel) + normalise(d) * 0.5f;
		_rot = rotation_of(_vel);
	}
	else {
		_target = get_entity_handle(find_enemy_near_point(_pos, 50.0f));
	}

	_vel = normalise(_vel) * _speed;

	if (entity_move_slide(this))
		destroy_entity(this);

	if (entity* e = find_enemy_near_line(_old_pos, _pos, _radius)) {
		if (e->_type == ET_DRONE) {
			fx_explosion(e->_pos, 0.5f, 3, rgba(1.0f, 0.0f, 0.0f, 0.0f), 0.5f);
			sound_play(sfx::DIT, -10.0f, 0.0f, 0);
			spawn_entity(new pickup, e->_pos);
			destroy_entity(e);
		}

		destroy_entity(this);
	}
}

void bullet::draw(draw_context* dc) {
	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->scale_xy(1.0f, 0.5f);
	dc->shape_outline(vec2(), 3, _radius * 1.3f, 0.0f, 0.5f, _colour);
}