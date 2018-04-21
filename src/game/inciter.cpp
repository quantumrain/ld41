#include "pch.h"
#include "game.h"

inciter::inciter() : entity(ET_INCITER) {
	_flags |= EF_BUILDING;
	_radius = 5.0f;
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

	dc->shape_outline(vec2(), 32, _radius, 0.0f, 0.5f, rgba());
}
