#include "pch.h"
#include "game.h"

hive::hive() : entity(ET_HIVE) {
	_flags |= EF_ENEMY | EF_BUILDING;
	_radius = 20.0f;
	_time = 0.0f;
	_rot = g_world.r.range(PI);
	_incited = 0;
	_num_spawned = 0;
	_anim = 0.0f;
	_anim_r = 1.0f;
}

void hive::init() {
	_time = g_world.r.range(1.0f, 10.0f);
	_anim = g_world.r.range(1.0f, 64.0f);
	_anim_r = g_world.r.range(0.75f, 1.5f);
}

void hive::tick() {
	int near_inciters = 0;

#if 0
	for_all([&](entity* e) {
		if (e->_type == ET_INCITER) {
			if (length_sq(e->_pos - _pos) < square(INCITER_RANGE))
				near_inciters++;
		}
	});

	_time -= DT * (1 + 2 * near_inciters);
#else
	_time -= DT;

	if (_incited > 0) {
		_time = min(_time - DT, 0.5f);
		near_inciters = 3;

		fx_circle(_pos, _radius * 1.25f, g_world.r.range(21, 27), 5.0f, rgba(0.25f, 0.75f, 0.75f, 1.0f));
	}
#endif

	if (_time < 0.0f) {
		_time += g_world.r.range(9.0f, 11.0f);

		int count = 0;

		for(auto& h : _drones) {
			if (auto* d = get_entity(h))
				count++;
		}

		int max_count = (near_inciters * 2) + 2;

		if (_incited > 0)
			_incited--;

		if (count < max_count) {
			vec2 o;

			do {
				o = g_world.r.range(_radius);
			} while(length_sq(o) < _radius);

			drone* d = spawn_entity(new drone(), _pos + o);

			if (_num_spawned++ > 15) {
				if (_num_spawned > 45) d->_angry = true;
				if (_num_spawned > 30) d->_angry = g_world.r.chance(1, 2);
				else                   d->_angry = g_world.r.chance(1, 5);
			}

			d->_hive = get_entity_handle(this);

			for(auto& h : _drones) {
				if (!get_entity(h)) {
					h = get_entity_handle(d);
					break;
				}
			}
		}		
	}

	_anim += DT * _anim_r;

	_vel *= 0.5f;
}

void hive::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	for(int i = 3; i >= 0; i--) {
		float f = i / 4.0f;
		float a = cosf(f + _anim * 0.2f) * 0.5f;
		float b = 1.0f + cosf(f * 2.0f + _anim * 0.2f) * 0.1f;

		if (i == 3)
			dc->shape(vec2(), 5, _radius * lerp(0.5f, 1.1f, f) * 1.25f * b + 1.0f, f * PI + a, rgba(0.09f, 0.03f, 0.01f, 1.0f));

		dc->shape_outline(vec2(), 5, _radius * lerp(0.5f, 1.1f, f) * 1.25f * b, f * PI + a, 0.5f, rgba(0.9f, 0.3f, 0.1f, 1.0f));
	}
}
