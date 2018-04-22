#include "pch.h"
#include "game.h"

generator::generator() : entity(ET_GENERATOR) {
	_flags |= EF_BUILDING;
	_radius = 10.0f;
	_colour = rgba(1.25f, 1.0f, 0.5f, 1.0f);
}

void generator::init() {
	g_world.ultimate = true;
}

void generator::tick() {
	_vel *= 0.5f;
}

void generator::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	float f = 0.0f;

	//if (_flash > 0.0f)
	//	f = max((0.1f - fmodf(_flash, 0.2f)) * 10.0f, 0.0f);

	rgba c = lerp(_colour, rgba(1.5f, 1.0f), f);

	//if (_lifetime < 1.0f)
	//	dc->shape_outline(vec2(), 64, lerp(0.0f, INCITER_RANGE, clamp(_lifetime * 2.0f, 0.0f, 1.0f)), 0.0f, 0.75f, _colour * lerp(0.75f, 0.0f, clamp((_lifetime - 0.5f) * 2.0f, 0.0f, 1.0f)));

	dc->shape(vec2(), 4, _radius + 1.0f, 0.0f, c * rgba(0.2f, 1.0f));
	dc->shape(vec2(), 4, _radius + 1.0f, PI * 0.25f, c * rgba(0.2f, 1.0f));

	dc->shape_outline(vec2(), 4, _radius, 0.0f, 0.5f, c);
	dc->shape_outline(vec2(), 4, _radius, PI * 0.25f, 0.5f, c);

	draw_string(*dc, vec2(), vec2(1.0f), TEXT_CENTRE | TEXT_VCENTRE, c, "U");
}
