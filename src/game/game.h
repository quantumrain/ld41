#pragma once

#include "base.h"
#include "sys.h"
#include "tile_map.h"

#define DT (1.0f / 60.0f)

struct world;

#define TURRET_RANGE           (120.0f)
#define COLLECTOR_RANGE        (160.0f)
#define INCITER_RANGE          (250.0f)
#define GENERATOR_RANGE	       (250.0f)

#define PLAYER_VULNERABLE_TIME (4.0f)

#define TURRET_COST    (8)
#define COLLECTOR_COST (16)
#define INCITER_COST   (64)
#define GENERATOR_COST (640)

enum entity_flag : u16 {
	EF_DESTROYED = 0x01,
	EF_PLAYER    = 0x02,
	EF_ENEMY     = 0x04,
	EF_BUILDING  = 0x08,
	EF_UNIT      = 0x10,
	EF_BULLET    = 0x20,
	EF_PICKUP    = 0x40,
};

enum entity_type : u16 {
	ET_PLAYER,
	ET_HIVE,
	ET_DRONE,
	ET_TURRET,
	ET_COLLECTOR,
	ET_GENERATOR,
	ET_INCITER,
	ET_BULLET,
	ET_PICKUP,
};

struct entity_handle {
	u16 index;
	u16 salt;

	explicit entity_handle() : index(0xFFFF), salt(0) { }
	explicit entity_handle(u16 index_, u16 salt_) : index(index_), salt(salt_) { }
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

	float _lifetime;
};

struct player : entity {
	player();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	void hit();
	void buy(entity_type item);

	float _time_since_hit;
};

struct hive : entity {
	hive();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	static const int MAX_DRONES = 16;

	float _time;
	int _incited;
	int _num_spawned;
	float _anim;
	float _anim_r;

	entity_handle _drones[MAX_DRONES];
};

struct drone : entity {
	drone();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	entity_handle _hive;
	float _time;
	float _aggressive;
	bool _angry;
	float _spot;
};

struct turret : entity {
	turret();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	float _time;
	float _fired;
};

struct collector : entity {
	collector();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	float _time;
	float _collected;
};

struct generator : entity {
	generator();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);
};

struct inciter : entity {
	inciter();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	float _time;
	float _flash;
};

struct bullet : entity {
	bullet();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	float _time;
	float _speed;
	entity_handle _target;
};

struct pickup : entity {
	pickup();

	virtual void init();
	virtual void tick();
	virtual void draw(draw_context* dc);

	float _time;
	entity_handle _target;
};

struct world {
	random r;

	list<entity> entities;
	array<entity*> handles;
	u16 salt;

	vec2 camera_pos;
	vec2 camera_vel;
	vec2 camera_target;

	vec2 view_size;
	mat44 view_proj;

	aabb2 limit;

	int resources;
	int total_resources;

	float flash_hotbar_turret;
	float flash_hotbar_collector;
	float flash_hotbar_inciter;
	float flash_hotbar_generator;

	float error_hotbar_turret;
	float error_hotbar_collector;
	float error_hotbar_inciter;
	float error_hotbar_generator;

	const char* message;
	float message_time;
	float message_max_time;

	bool ultimate;
	int num_turrets;

	world();
};

extern world g_world;

entity* spawn_entity(entity* e, vec2 initial_pos);
void destroy_entity(entity* e);

entity* get_entity(entity_handle h);
entity_handle get_entity_handle(entity* e);

int entity_move_slide(entity* e);

player* get_player();

entity* find_entity_near_point(vec2 pos, float range, entity_type type);
entity* find_building_near_point(vec2 pos, float range);
entity* find_enemy_near_point(vec2 pos, float range);
entity* find_enemy_near_line(vec2 from, vec2 to, float r);
void avoid_crowd(world* w, entity* self);
void avoid_buildings(world* w, entity* self);

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
void fx_circle(vec2 pos, float radius, int count, float wobble, rgba c, int dur_mult = 1);
void fx_message(const char* message, float time = 1.5f);

float get_vol(vec2 pos);