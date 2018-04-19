#include "pch.h"
#include "game.h"

tile::tile() : id() {
}

tile_map::tile_map() : _map(), _w(), _h() { }
tile_map::~tile_map() { delete [] _map; }

void tile_map::init(int w, int h) {
	delete [] _map;

	_map = new tile[w * h];
	_w = w;
	_h = h;
}

namespace {
	bool raycast_aabb(vec2 p, vec2 d, aabb2 a, hit_result* hr) {
		float tmin = 0.0f;
		float tmax = 1.0f;

		side s = side::NONE;

		for(int i = 0; i < 2; i++) {
			if (fabsf(d[i]) < 0.0000001f) {
				if (p[i] < a.min[i] || p[i] > a.max[i]) return false;
			}
			else {
				float ood = 1.0f / d[i];
				float t1 = (a.min[i] - p[i]) * ood;
				float t2 = (a.max[i] - p[i]) * ood;

				if (t1 > t2) {
					if (t2 > tmin) { tmin = t2; s = (side)(1 + i * 2); }
					if (t1 < tmax) { tmax = t1; }
				}
				else {
					if (t1 > tmin) { tmin = t1; s = (side)(i * 2); }
					if (t2 < tmax) { tmax = t2; }
				}

				if (tmin > tmax) return false;
			}
		}

		if (tmin >= hr->t) return false;

		hr->hit		= p + d * tmin;
		hr->t		= tmin;
		hr->side	= s;

		return true;
	}
}

hit_result tile_map::raycast(vec2 from, vec2 to) {
	vec2i cur(ifloor(from));
	vec2i end(ifloor(to));
	vec2i sign(isign(to.x - from.x), isign(to.y - from.y));
	vec2 abs_delta(fabsf(to.x - from.x), fabsf(to.y - from.y));
	vec2 delta_t(vec2(1.0f) / abs_delta);
	vec2 lb(to_vec2(cur));
	vec2 ub(lb + vec2(1.0f));
	vec2 t(vec2((from.x > to.x) ? (from.x - lb.x) : (ub.x - from.x), (from.y > to.y) ? (from.y - lb.y) : (ub.y - from.y)) / abs_delta);

	for(;;) {
		if (is_raycast_solid(cur.x, cur.y)) {
			hit_result hr;
			if (raycast_aabb(from, to - from, get_tile_bb(cur.x, cur.y), &hr)) {
				hr.pos = cur;
				hr.id = get_id(cur.x, cur.y);
				return hr;
			}
		}

		if (t.x <= t.y) {
			if (cur.x == end.x) break;
			t.x += delta_t.x;
			cur.x += sign.x;
		}
		else {
			if (cur.y == end.y) break;
			t.y += delta_t.y;
			cur.y += sign.y;
		}
	}

	return hit_result();
}

namespace {
	float clip(float min0, float max0, float min1, float max1, float delta) {
		if ((delta > 0.0f) && (min0 >= max1)) return min(delta, min0 - max1);
		if ((delta < 0.0f) && (max0 <= min1)) return max(delta, max0 - min1);
		return delta;
	}

	float clip_x(const aabb2& a, const aabb2& b, float vx) { return overlaps_y(a, b) ? clip(a.min.x, a.max.x, b.min.x, b.max.x, vx) : vx; }
	float clip_y(const aabb2& a, const aabb2& b, float vy) { return overlaps_x(a, b) ? clip(a.min.y, a.max.y, b.min.y, b.max.y, vy) : vy; }

	template<float clip_t(const aabb2&, const aabb2&, float)> float slide_t(tile_map* tm, int x0, int y0, int x1, int y1, const aabb2& self, float delta) {
		for(int x = x0; x <= x1; x++) {
			for(int y = y0; y <= y1; y++) {
				if (tm->is_slide_solid(x, y)) {
					delta = clip_t(tm->get_tile_bb(x, y), self, delta);
				}
			}
		}

		return delta;
	}
}

aabb2 tile_map::slide(aabb2 self, vec2 delta, int* clipped) {
	aabb2 bounds(offset_enclosing(self, delta));

	int x0 = ifloor(bounds.min.x);
	int y0 = ifloor(bounds.min.y);
	int x1 = ifloor(bounds.max.x);
	int y1 = ifloor(bounds.max.y);

	float dx = slide_t<clip_x>(this, x0, y0, x1, y1, self, delta.x);

	self.min.x += dx;
	self.max.x += dx;

	float dy = slide_t<clip_y>(this, x0, y0, x1, y1, self, delta.y);

	self.min.y += dy;
	self.max.y += dy;

	if (clipped) {
		*clipped = 0;
		if (delta.x != dx) *clipped |= (delta.x > 0.0f) ? CLIPPED_XP : CLIPPED_XN;
		if (delta.y != dy) *clipped |= (delta.y > 0.0f) ? CLIPPED_YP : CLIPPED_YN;
	}

	return self;
}

void tile_map::set_id(int x, int y, int id) {
	if (tile* t = get_tile(x, y))
		t->id = id;
}

tile* tile_map::get_tile(int x, int y) {
	if ((x >= 0) && (y >= 0) && (x < _w) && (y < _h))
		return &_map[x + (y * _w)];
	return 0;
}

int tile_map::get_id(int x, int y) {
	if (tile* t = get_tile(x, y))
		return t->id;
	return 0;
}

bool tile_map::is_raycast_solid(int x, int y) {
	return get_id(x, y) != 0;
}

bool tile_map::is_slide_solid(int x, int y) {
	return get_id(x, y) != 0;
}

aabb2 tile_map::get_tile_bb(int x, int y) {
	return aabb2 { { (float)x, (float)y }, { (float)x + 1.0f, (float)y + 1.0f } };
}