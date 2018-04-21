#include "pch.h"
#include "game.h"

pickup::pickup() : entity(ET_PICKUP) {
	_flags |= EF_PICKUP;
	_radius = 1.75f;
	_colour = rgba(0.75f, 1.0f, 0.0f, 1.0f);
	_time = 10.0f;
}

void pickup::init() {
	_vel = g_world.r.range(vec2(30.0f));
}

void pickup::tick() {
	_time -= DT;

	if (_time <= 0.0f)
		destroy_entity(this);

	if (player* p = get_player()) {
		vec2 d = p->_pos - _pos;
		float l = length_sq(d);

		if (l < square(_radius + p->_radius)) {
			sound_play(sfx::DIT, 0.0f, 0.0f, 0);
			g_world.resources++;
			destroy_entity(this);
		}
		else if (l < square(48.0f)) {
			_vel += d * (80.0f / sqrtf(l));
			_time = max(_time, 1.0f);
		}
		else
		{
			if (entity* e = get_entity(_target)) {
				vec2 d = e->_pos - _pos;
				float l = length_sq(d);

				if (l < square(_radius + e->_radius)) {
					sound_play(sfx::DIT, 0.0f, 0.0f, 0);
					g_world.resources++;
					destroy_entity(this);
				}
				else {
					_vel += d * (40.0f / sqrtf(l));
					_time = max(_time, 1.0f);
				}
			}
		}
	}

	entity_move_slide(this);

	avoid_buildings(&g_world, this);

	if (length_sq(_vel) > 30.0f)
		_vel *= 0.8f;
}

void pickup::draw(draw_context* dc) {
	dc->translate(_pos);

	dc->shape(vec2(), 8, _radius * 2.0f, 0.0f, _colour);
	draw_string(*dc, vec2(), vec2(0.5f), TEXT_CENTRE | TEXT_VCENTRE, rgba(0.0f, 1.0f), "$");
}
