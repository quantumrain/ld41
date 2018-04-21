#include "pch.h"
#include "game.h"

collector::collector() : entity(ET_COLLECTOR) {
	_flags |= EF_BUILDING;
	_radius = 7.5f;
	_colour = rgba(0.5f, 0.1f, 1.0f, 1.0f);
	_time = 0.0f;
}

void collector::init() {
}

void collector::tick() {
	_time -= DT;

	if (_time < 0.0f) {
		_time += 2.0f;

		if (entity* e = find_entity_near_point(_pos, 80.0f, ET_PICKUP)) {
			pickup* p = (pickup*)e;
			p->_target = get_entity_handle(this);
		}
	}

	_vel *= 0.5f;
}

void collector::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	rgba c = lerp(_colour, rgba(1.5f, 1.0f), max((0.1f - _time) * 10.0f, 0.0f));

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, c);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, c);

	if (_lifetime < 3.0f) {
		dc->shape_outline(vec2(), 64, lerp(0.0f, 80.0f, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.25f, _colour * lerp(0.25f, 0.0f, clamp((_lifetime - 1.0f) / 2.0f, 0.0f, 1.0f)));
	}

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, c, "C");
}
