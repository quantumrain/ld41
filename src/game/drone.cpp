#include "pch.h"
#include "game.h"

drone::drone() : entity(ET_DRONE) {
	_flags |= EF_ENEMY | EF_UNIT;
	_radius = 5.0f;
}

void drone::init() {
}

void drone::tick() {
	_vel *= 0.5f;
}

void drone::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 32, _radius, 0.0f, 0.5f, rgba());
}
