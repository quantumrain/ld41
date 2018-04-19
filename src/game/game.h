#pragma once

#include "base.h"
#include "sys.h"
#include "tile_map.h"

#define DT (1.0f / 60.0f)

struct world;

enum entity_flag : u16 {
	EF_DESTROYED = 0x01,
	EF_PLAYER    = 0x02,
	EF_ENEMY     = 0x04,
};

enum entity_type : u16 {
	ET_PLAYER,
};

struct entity_handle {
	u16 index;

	explicit entity_handle(u16 index_ = 0xFFFF) : index(index_) { }
};

struct entity {
	entity(entity_type type);
	virtual ~entity();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	u16           _flags;
	entity_type   _type;
	entity_handle _handle_internal;

	vec2 _pos;
	vec2 _old_pos;
	vec2 _vel;
	f32  _rot;
	f32  _radius;
	rgba _colour;
};

struct player : entity {
	player();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);
};

struct world {
	random r;

	list<entity> entities;
	array<entity*> handles;

	vec2 camera_pos;
	vec2 camera_vel;
	vec2 camera_target;
	vec2 camera_old_target;

	vec2 view_size;
	mat44 view_proj;

	world();
};

extern world g_world;

entity* spawn_entity(entity* e, vec2 initial_pos);
void destroy_entity(entity* e);

entity* get_entity(entity_handle h);
entity_handle get_entity_handle(entity* e);

int entity_move_slide(entity* e);

player* get_player();

entity* find_enemy_near_line(vec2 from, vec2 to, float r);

void world_tick(bool paused);
void world_draw(draw_context* dc);

template<typename T> T* spawn_entity(T* e, vec2 initial_pos) {
	return (T*)spawn_entity(static_cast<entity*>(e), initial_pos);
}

template<typename F> void for_all(F&& f) {
	auto& entities = g_world.entities;

	for(int i = 0; i < entities.size(); i++) {
		entity* e = entities[i];

		if (!(e->_flags & EF_DESTROYED))
			f(e);
	}
}

void psys_init(int max_particles);
void psys_update();
void psys_render(draw_context* dc);

void psys_spawn(vec2 pos, vec2 vel, float damp, float size0, float size1, float rot_v, rgba c, int lifetime);
void fx_explosion(vec2 pos, float strength, int count, rgba c, float psize);
