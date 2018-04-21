#include "pch.h"
#include "game.h"

hive::hive() : entity(ET_HIVE) {
	_flags |= EF_ENEMY;
	_radius = 15.0f;
}

void hive::init() {
}

void hive::tick() {
	_vel *= 0.5f;
}

void hive::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	dc->shape_outline(vec2(), 8, _radius, 0.0f, 0.5f, rgba(0.9f, 0.3f, 0.1f, 1.0f));
}
