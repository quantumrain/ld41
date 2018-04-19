#ifndef TILE_MAP_H
#define TILE_MAP_H

enum {
	CLIPPED_XN = 1,
	CLIPPED_XP = 2,
	CLIPPED_X = CLIPPED_XN | CLIPPED_XP,
	CLIPPED_YN = 4,
	CLIPPED_YP = 8,
	CLIPPED_Y = CLIPPED_YN | CLIPPED_YP
};

enum class side { NONE, XN, XP, YN, YP };

struct hit_result {
	vec2i pos;
	vec2 hit;
	float t;
	int id;
	side side;

	hit_result() : t(1.0f), id(), side(side::NONE) { }

	explicit operator bool() const { return id != 0; }
};

struct tile {
	u8 id;

	tile();
};

struct tile_map {
	tile_map();
	~tile_map();

	void init(int w, int h);

	hit_result raycast(vec2 from, vec2 to);
	aabb2 slide(aabb2 self, vec2 delta, int* clipped);

	void set_id(int x, int y, int id);

	tile* get_tile(int x, int y);
	int get_id(int x, int y);

	bool is_raycast_solid(int x, int y);
	bool is_slide_solid(int x, int y);

	aabb2 get_tile_bb(int x, int y);

	tile* _map;
	int _w, _h;
};

#endif // TILE_MAP_H