#include "pch.h"
#include "game.h"

inciter::inciter() : entity(ET_INCITER) {
	_flags |= EF_BUILDING;
	_radius = 15.0f;
	_colour = rgba(0.35f, 1.0f, 1.25f, 1.0f);
	_time = 0.0f;
}

void inciter::init() {
	_time = g_world.r.range(1.0f, 4.0f);

	fx_circle(_pos, INCITER_RANGE, 512, 1.0f, _colour * rgba(0.5f, 1.0f), 4);
	fx_circle(_pos, INCITER_RANGE, 512, 1.0f, _colour, 4);
}

void inciter::tick() {
	_time -= DT;
	_flash -= DT;

	if (_time < 0.0f) {
		_time += g_world.r.range(3.5f, 4.5f);

		const int MAX_HIVES = 64;
		entity* hives[MAX_HIVES];
		int num_hives = 0;

		for_all([&](entity* e) {
			if (e->_type == ET_HIVE) {
				if (length_sq(e->_pos - _pos) < square(INCITER_RANGE)) {
					if (num_hives < MAX_HIVES)
						hives[num_hives++] = e;
				}
			}
		});

		if (num_hives > 0) {
			hive* h = (hive*)hives[g_world.r.rand(num_hives)];
			h->_incited = 6;
			_flash = 1.5f;
		}
	}

	_vel *= 0.5f;
}

void inciter::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	float f = 0.0f;

	if (_flash > 0.0f)
		f = max((0.1f - fmodf(_flash, 0.2f)) * 10.0f, 0.0f);

	rgba c = lerp(_colour, rgba(1.5f, 1.0f), f);

	//if (_lifetime < 1.0f)
	//	dc->shape_outline(vec2(), 64, lerp(0.0f, INCITER_RANGE, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.75f, _colour * lerp(0.75f, 0.0f, clamp((_lifetime - 0.5f) * 2.0f, 0.0f, 1.0f)));

	dc->shape(vec2(), 4, _radius + 1.0f, 0.0f, c * rgba(0.2f, 1.0f));
	dc->shape(vec2(), 4, _radius + 1.0f, PI * 0.25f, c * rgba(0.2f, 1.0f));

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, c);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, c);

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, c, "X");
}
