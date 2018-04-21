#include "pch.h"
#include "game.h"

turret::turret() : entity(ET_TURRET) {
	_flags |= EF_BUILDING;
	_radius = 10.0f;
}

void turret::init() {
}

void turret::tick() {
	_vel *= 0.5f;
}

void turret::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, rgba(0.85f, 0.5f, 0.1f, 1.0f));
}
