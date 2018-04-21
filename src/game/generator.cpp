#include "pch.h"
#include "game.h"

generator::generator() : entity(ET_GENERATOR) {
	_flags |= EF_BUILDING;
	_radius = 5.0f;
}

void generator::init() {
}

void generator::tick() {
	_vel *= 0.5f;
}

void generator::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 32, _radius, 0.0f, 0.5f, rgba());
}
