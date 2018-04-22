#include "pch.h"
#include "game.h"

world g_world;

// world

world::world()
	: salt()
	, resources()
	, total_resources()
	, flash_hotbar_turret()
	, flash_hotbar_collector()
	, flash_hotbar_inciter()
	, error_hotbar_turret()
	, error_hotbar_collector()
	, error_hotbar_inciter()
	, message()
	, message_time()
{ }

// entity

entity::entity(entity_type type) : _flags(), _type(type), _rot(0.0f), _radius(8.0f), _lifetime() { }
entity::~entity() { }

void entity::init() { }
void entity::tick() { }
void entity::draw(draw_context* dc) { dc->shape_outline(_pos, 8, _radius, _rot, 0.5f, _colour); }

// etc

entity* spawn_entity(entity* e, vec2 initial_pos) {
	world* w = &g_world;

	if (e) {
		w->entities.push_back(e);

		e->_pos     = initial_pos;
		e->_old_pos = initial_pos;

		e->init();
	}

	return e;
}

void destroy_entity(entity* e) {
	if (e)
		e->_flags |= EF_DESTROYED;
}

entity* get_entity(entity_handle h) {
	world* w = &g_world;

	if (entity** ep = w->handles.at(h.index)) {
		if (entity* e = *ep) {
			if ((e->_handle_internal.index == h.index) && (e->_handle_internal.salt == h.salt))
				return e;
		}
	}

	return 0;
}

entity_handle get_entity_handle(entity* e) {
	world* w = &g_world;

	if (e) {
		if (get_entity(e->_handle_internal))
			return e->_handle_internal;

		for(int i = 0; i < w->handles.size(); i++) {
			if (!w->handles[i]) {
				w->handles[i] = e;
				e->_handle_internal = entity_handle(i, ++w->salt);
				return e->_handle_internal;
			}
		}

		if (w->handles.push_back(e)) {
			e->_handle_internal = entity_handle(w->handles.size() - 1, ++w->salt);
			return e->_handle_internal;
		}
	}

	return entity_handle();
}

void entity_prune() {
	world* w = &g_world;

	for(int i = 0; i < w->entities.size(); ) {
		if (w->entities[i]->_flags & EF_DESTROYED) {
			int h = w->entities[i]->_handle_internal.index;

			if (h < w->handles.size()) {
				assert(w->handles[h] == w->entities[i]);
				w->handles[h] = 0;
			}

			w->entities.swap_remove(i);
		}
		else
			i++;
	}
}

int entity_move_slide(entity* e) {
	int clipped = 0;

	e->_old_pos = e->_pos;
	e->_pos += e->_vel * DT;

	if (e->_pos.x < g_world.limit.min.x) { e->_pos.x = g_world.limit.min.x; clipped |= CLIPPED_XN; }
	if (e->_pos.x > g_world.limit.max.x) { e->_pos.x = g_world.limit.max.x; clipped |= CLIPPED_XP; }
	if (e->_pos.y < g_world.limit.min.y) { e->_pos.y = g_world.limit.min.y; clipped |= CLIPPED_YN; }
	if (e->_pos.y > g_world.limit.max.y) { e->_pos.y = g_world.limit.max.y; clipped |= CLIPPED_YP; }

	return clipped;
}

player* get_player() {
	for(auto* e : g_world.entities) {
		if (e->_type == ET_PLAYER)
			return (player*)e;
	}

	return 0;
}

entity* find_entity_near_point(vec2 pos, float range, entity_type type) {
	entity* best   = 0;
	float   best_t = square(range);

	for(auto* e : g_world.entities) {
		if (e->_flags & EF_DESTROYED)
			continue;

		if (e->_type != type)
			continue;

		float t = length_sq(e->_pos - pos);

		if (t < best_t) {
			best   = e;
			best_t = t;
		}
	}

	return best;
}

entity* find_building_near_point(vec2 pos, float range) {
	entity* best   = 0;
	float   best_t = range;

	for(auto* e : g_world.entities) {
		if (e->_flags & EF_DESTROYED)
			continue;

		if (!(e->_flags & EF_BUILDING))
			continue;

		float r = e->_radius;

		if (e->_type == ET_HIVE)
			r *= 0.75f;

		float t = length(e->_pos - pos) - r;

		if (t < best_t) {
			best   = e;
			best_t = t;
		}
	}

	return best;
}

entity* find_enemy_near_point(vec2 pos, float range) {
	entity* best   = 0;
	float   best_t = square(range);

	for(auto* e : g_world.entities) {
		if (e->_flags & (EF_DESTROYED | EF_BUILDING))
			continue;

		if ((e->_flags & (EF_ENEMY | EF_UNIT)) != (EF_ENEMY | EF_UNIT))
			continue;

		float t = length_sq(e->_pos - pos);

		if (t < best_t) {
			best   = e;
			best_t = t;
		}
	}

	return best;
}

entity* find_enemy_near_line(vec2 from, vec2 to, float r) {
	entity* best   = 0;
	float   best_t = FLT_MAX;

	vec2	d = to - from;
	float	l = length(d);

	if (l <= 0.0f)
		l = 1.0f;

	d /= l;

	for(auto* e : g_world.entities) {
		if (e->_flags & EF_DESTROYED)
			continue;

		if ((e->_flags & (EF_ENEMY | EF_UNIT)) != (EF_ENEMY | EF_UNIT))
			continue;

		float t = clamp(dot((e->_pos - from), d), 0.0f, l);
		vec2  p = from + d * t;
		float s = length_sq(e->_pos - p);

		if (s < square(e->_radius + r)) {
			if (t < best_t) {
				best   = e;
				best_t = t;
			}
		}
	}

	return best;
}

void avoid_crowd(world* w, entity* self) {
	auto self_pos    = self->_pos;
	auto self_radius = self->_radius;
	auto self_type   = self->_type;

	for(auto e : w->entities) {
		if (e == self)
			continue;

		if (e->_flags & EF_DESTROYED)
			continue;

		if (e->_type != self_type)
			continue;

		vec2	d		= self_pos - e->_pos;
		float	min_l	= self_radius + e->_radius;

		if (length_sq(d) > square(min_l))
			continue;

		float l = length(d);

		if (l < 0.0001f)
			d = vec2(1.0f, 0.0f);
		else
			d /= l;

		float force_self	= 8.0f;
		float force_e		= 8.0f;

		self->_vel += d * force_self;
		e->_vel -= d * force_e;
	}
}

void avoid_buildings(world* w, entity* self) {
	auto self_pos    = self->_pos;
	auto self_radius = self->_radius;

	for(auto e : w->entities) {
		if (e == self)
			continue;

		if (e->_flags & EF_DESTROYED)
			continue;

		if (!(e->_flags & EF_BUILDING))
			continue;

		vec2	d		= self_pos - e->_pos;
		float	min_l	= self_radius + e->_radius;

		if (length_sq(d) > square(min_l))
			continue;

		if (length_sq(d) < 1.0f)
			continue;

		self->_pos = e->_pos + normalise(d) * min_l;
	}
}

void world_tick(bool paused) {
	world* w = &g_world;

	if (!paused) {
		for_all([](entity* e) { 
			e->_lifetime += DT;
			e->tick();
		});
	}

	entity_prune();
}

void world_draw(draw_context* dc) {
	world* w = &g_world;

	for(int y = -6; y <= 6; y++) {
		for(int x = -6; x <= 6; x++) {
			random r((x * 100) + y);

			for(int i = 0; i < 10; i++) {
				dc->shape(vec2(150.0f * x, 150.0f * y) + r.range(vec2(75.0f)), 4, r.range(40.0f, 60.0f), r.range(PI), rgba(0.04f, 0.05f, 0.1f, 0.0f));
			}
		}
	}

	//dc->rect_outline(w->limit.min, w->limit.max, 0.5f, rgba(0.1f, 1.0f));

	psys_render(&dc->copy());

	for_all([dc](entity* e) { if (!(e->_flags & EF_PLAYER)) e->draw(&dc->copy()); });
	for_all([dc](entity* e) { if (e->_flags & EF_PLAYER) e->draw(&dc->copy()); });
}
