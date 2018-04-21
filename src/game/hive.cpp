#include "pch.h"
#include "game.h"

hive::hive() : entity(ET_HIVE) {
	_flags |= EF_ENEMY | EF_BUILDING;
	_radius = 20.0f;
	_time = 0.0f;
	_rot = g_world.r.range(PI);
}

void hive::init() {
	_time = g_world.r.range(1.0f, 10.0f);
}

void hive::tick() {
	int near_inciters = 0;

	for_all([&](entity* e) {
		if (e->_type == ET_INCITER) {
			if (length_sq(e->_pos - _pos) < square(250.0f))
				near_inciters++;
		}
	});

	_time -= DT * (1 + 2 * near_inciters);

	if (_time < 0.0f) {
		_time += g_world.r.range(9.0f, 11.0f);

		int count = 0;

		for(auto& h : _drones) {
			if (auto* d = get_entity(h))
				count++;
		}

		int max_count = (near_inciters * 2) + 2;

		if (count < max_count) {
			drone* d = spawn_entity(new drone(), _pos + g_world.r.range(_radius));

			d->_hive = get_entity_handle(this);

			for(auto& h : _drones) {
				if (!get_entity(h)) {
					h = get_entity_handle(d);
					break;
				}
			}
		}		
	}

	_vel *= 0.5f;
}

void hive::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	for(int i = 0; i < 4; i++) {
		float f = i / 4.0f;

		dc->shape_outline(vec2(), 5, _radius * lerp(0.5f, 1.1f, f) * 1.25f, f * PI, 0.5f, rgba(0.9f, 0.3f, 0.1f, 1.0f));
	}
}
