#include "pch.h"
#include "game.h"

inciter::inciter() : entity(ET_INCITER) {
	_flags |= EF_BUILDING;
	_radius = 15.0f;
	_colour = rgba(0.25f, 0.75f, 0.75f, 1.0f);
}

void inciter::init() {
}

void inciter::tick() {
	_vel *= 0.5f;
}

void inciter::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, _colour);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, _colour);

	if (_lifetime < 3.0f) {
		dc->shape_outline(vec2(), 64, lerp(0.0f, 250.0f, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.25f, _colour * lerp(0.25f, 0.0f, clamp((_lifetime - 1.0f) / 2.0f, 0.0f, 1.0f)));
	}

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, _colour, "I");
}
