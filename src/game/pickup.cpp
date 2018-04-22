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

	if (g_world.resources == 0)
		fx_message("collect $");
}

void pickup::tick() {
	_time -= DT;

	if (_time <= 0.0f)
		destroy_entity(this);

	if (player* p = get_player()) {
		vec2 d = p->_pos - _pos;
		float l = length_sq(d);

		if (p->_time_since_hit > 0.5f)
			l = square(100.0f);

		auto collect = [&]() {
				sound_play(sfx::PICKUP, 0.0f, get_vol(_pos) - 6.0f, 0);
				g_world.resources++;
				g_world.total_resources++;
				destroy_entity(this);

				if (g_world.resources == TURRET_COST   ) sound_play(sfx::TURRET_ACQ, 0.0f, 3.0f);
				if (g_world.resources == COLLECTOR_COST) sound_play(sfx::COLLECTOR_ACQ, 0.0f, 3.0f);
				if (g_world.resources == INCITER_COST  ) sound_play(sfx::INCITER_ACQ, 0.0f, 3.0f);
				if (g_world.resources == GENERATOR_COST) sound_play(sfx::GENERATOR_ACQ, 0.0f, 3.0f);

				if ((g_world.num_turrets <= 1) && (g_world.resources >= TURRET_COST))
					fx_message("press [1] to deploy <T>", 3.0f);
			};

		if (l < square(_radius + p->_radius)) {
			collect();
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

				if (l < square(_radius + e->_radius + 1.0f)) {
					if (e->_type == ET_COLLECTOR) {
						((collector*)e)->_collected = 0.25f;
					}

					collect();
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
