#include "pch.h"
#include "game.h"

vec2 drone_vel() {
	return g_world.r.range(vec2(30.0f)) + g_world.r.range(vec2(15.0f));
}

drone::drone() : entity(ET_DRONE) {
	_flags |= EF_ENEMY | EF_UNIT;
	_radius = 5.0f;
	_time = 0.0f;
	_aggressive = 0.0f;
	_angry = false;
}

void drone::init() {
	_vel = drone_vel();
	_time = g_world.r.range(5.0f, 15.0f);
}

void drone::tick() {
	if (_aggressive > 0.0f) {
		if (_aggressive < 1.0f) {
			if (_aggressive < 0.1f) {
				if (player* pl = get_player()) {
					if (length_sq(pl->_pos - _pos) < square(150.0f)) {
						_aggressive = g_world.r.range(1.0f, 2.0f);
					}
				}
			}
		}
		else {
			if (player* pl = get_player()) {
				vec2 d = pl->_pos - _pos;

				_vel += normalise(d) * (_angry ? 30.0f : 20.0f);
				_vel *= 0.9f;
			}
		}

		_aggressive -= DT;
	}
	else {
		if (entity* h = get_entity(_hive)) {
			vec2 d = h->_pos - _pos;

			if (length_sq(d) > square(50.0f)) {
				if (dot(_vel, d) < 0.0f) {
					_vel = drone_vel();
				}
			}

			_time -= DT;

			if (_time < 0.0f) {
				_time += g_world.r.range(5.0f, 15.0f);
				_vel = drone_vel();
			}
		}

		if (player* pl = get_player()) {
			if (length_sq(pl->_pos - _pos) < square(120.0f)) {
				_aggressive = g_world.r.range(3.0f, 6.0f);
				sound_play(sfx::DIT, 10.0f, 0.0f, 0);
			}

			if (_angry || g_world.r.chance(1, 100000)) {
				_aggressive = g_world.r.range(3.0f, 6.0f);
				_angry = true;
				_radius = 7.5f;
			}
		}
	}

	if (player* pl = get_player()) {
		if (length_sq(pl->_pos - _pos) < square(_radius + pl->_radius * 0.75f)) {
			sound_play(sfx::DIT, -30.0f, 9.0f, 0);
			sound_play(sfx::DIT, -40.0f, 6.0f, 0);
			sound_play(sfx::DIT, -50.0f, 3.0f, 0);
			fx_explosion(pl->_pos, 1.0f, 40, rgba(), 1.0f);
			destroy_entity(pl);
		}
	}

	avoid_crowd(&g_world, this);
	avoid_buildings(&g_world, this);
	entity_move_slide(this);
}

void drone::draw(draw_context* dc) {
	draw_context dcc = dc->copy();

	dc->translate(_pos);
	dc->rotate_z(_rot);

	rgba c = lerp(rgba(1.0f, 0.3f, 0.7f, 1.0f), rgba(1.25f, 0.4f, 0.1f, 1.0f), clamp(_aggressive, 0.0f, 1.0f));

	dc->shape_outline(vec2(), 3, _radius, 0.0f, 0.5f, c);
	dc->shape_outline(vec2(), 3, _radius, PI, 0.5f, c * rgba(0.8f, 1.0f));
}
