#pragma once

#define VIMPL_COMP(a_, b_, comp_, op_)	(((a_).comp_) op_ ((b_).comp_))
#define VIMPL_OP2(a_, b_, op_)			{ VIMPL_COMP(a_, b_, x, op_), VIMPL_COMP(a_, b_, y, op_) }
#define VIMPL_OP3(a_, b_, op_)			{ VIMPL_COMP(a_, b_, x, op_), VIMPL_COMP(a_, b_, y, op_), VIMPL_COMP(a_, b_, z, op_) }
#define VIMPL_OP4(a_, b_, op_)			{ VIMPL_COMP(a_, b_, x, op_), VIMPL_COMP(a_, b_, y, op_), VIMPL_COMP(a_, b_, z, op_), VIMPL_COMP(a_, b_, w, op_) }

// vec*

inline vec2 operator+(const vec2& a, const vec2& b) { return VIMPL_OP2(a, b, +); }
inline vec3 operator+(const vec3& a, const vec3& b) { return VIMPL_OP3(a, b, +); }
inline vec4 operator+(const vec4& a, const vec4& b) { return VIMPL_OP4(a, b, +); }
inline vec2 operator-(const vec2& a, const vec2& b) { return VIMPL_OP2(a, b, -); }
inline vec3 operator-(const vec3& a, const vec3& b) { return VIMPL_OP3(a, b, -); }
inline vec4 operator-(const vec4& a, const vec4& b) { return VIMPL_OP4(a, b, -); }
inline vec2 operator*(const vec2& a, const vec2& b) { return VIMPL_OP2(a, b, *); }
inline vec3 operator*(const vec3& a, const vec3& b) { return VIMPL_OP3(a, b, *); }
inline vec4 operator*(const vec4& a, const vec4& b) { return VIMPL_OP4(a, b, *); }
inline vec2 operator/(const vec2& a, const vec2& b) { return VIMPL_OP2(a, b, /); }
inline vec3 operator/(const vec3& a, const vec3& b) { return VIMPL_OP3(a, b, /); }
inline vec4 operator/(const vec4& a, const vec4& b) { return VIMPL_OP4(a, b, /); }

inline vec2 operator+(const vec2& a) { return a; }
inline vec3 operator+(const vec3& a) { return a; }
inline vec4 operator+(const vec4& a) { return a; }
inline vec2 operator-(const vec2& a) { return { -a.x, -a.y }; }
inline vec3 operator-(const vec3& a) { return { -a.x, -a.y, -a.z }; }
inline vec4 operator-(const vec4& a) { return { -a.x, -a.y, -a.z, -a.w }; }

inline vec2& operator+=(vec2& a, const vec2& b) { a = a + b; return a; }
inline vec3& operator+=(vec3& a, const vec3& b) { a = a + b; return a; }
inline vec4& operator+=(vec4& a, const vec4& b) { a = a + b; return a; }
inline vec2& operator-=(vec2& a, const vec2& b) { a = a - b; return a; }
inline vec3& operator-=(vec3& a, const vec3& b) { a = a - b; return a; }
inline vec4& operator-=(vec4& a, const vec4& b) { a = a - b; return a; }
inline vec2& operator*=(vec2& a, const vec2& b) { a = a * b; return a; }
inline vec3& operator*=(vec3& a, const vec3& b) { a = a * b; return a; }
inline vec4& operator*=(vec4& a, const vec4& b) { a = a * b; return a; }
inline vec2& operator/=(vec2& a, const vec2& b) { a = a / b; return a; }
inline vec3& operator/=(vec3& a, const vec3& b) { a = a / b; return a; }
inline vec4& operator/=(vec4& a, const vec4& b) { a = a / b; return a; }

// vec*i

inline vec2i operator+(const vec2i& a, const vec2i& b) { return VIMPL_OP2(a, b, +); }
inline vec3i operator+(const vec3i& a, const vec3i& b) { return VIMPL_OP3(a, b, +); }
inline vec4i operator+(const vec4i& a, const vec4i& b) { return VIMPL_OP4(a, b, +); }
inline vec2i operator-(const vec2i& a, const vec2i& b) { return VIMPL_OP2(a, b, -); }
inline vec3i operator-(const vec3i& a, const vec3i& b) { return VIMPL_OP3(a, b, -); }
inline vec4i operator-(const vec4i& a, const vec4i& b) { return VIMPL_OP4(a, b, -); }
inline vec2i operator*(const vec2i& a, const vec2i& b) { return VIMPL_OP2(a, b, *); }
inline vec3i operator*(const vec3i& a, const vec3i& b) { return VIMPL_OP3(a, b, *); }
inline vec4i operator*(const vec4i& a, const vec4i& b) { return VIMPL_OP4(a, b, *); }
inline vec2i operator/(const vec2i& a, const vec2i& b) { return VIMPL_OP2(a, b, /); }
inline vec3i operator/(const vec3i& a, const vec3i& b) { return VIMPL_OP3(a, b, /); }
inline vec4i operator/(const vec4i& a, const vec4i& b) { return VIMPL_OP4(a, b, /); }

inline vec2i operator+(const vec2i& a) { return a; }
inline vec3i operator+(const vec3i& a) { return a; }
inline vec4i operator+(const vec4i& a) { return a; }
inline vec2i operator-(const vec2i& a) { return { -a.x, -a.y }; }
inline vec3i operator-(const vec3i& a) { return { -a.x, -a.y, -a.z }; }
inline vec4i operator-(const vec4i& a) { return { -a.x, -a.y, -a.z, -a.w }; }

inline vec2i& operator+=(vec2i& a, const vec2i& b) { a = a + b; return a; }
inline vec3i& operator+=(vec3i& a, const vec3i& b) { a = a + b; return a; }
inline vec4i& operator+=(vec4i& a, const vec4i& b) { a = a + b; return a; }
inline vec2i& operator-=(vec2i& a, const vec2i& b) { a = a - b; return a; }
inline vec3i& operator-=(vec3i& a, const vec3i& b) { a = a - b; return a; }
inline vec4i& operator-=(vec4i& a, const vec4i& b) { a = a - b; return a; }
inline vec2i& operator*=(vec2i& a, const vec2i& b) { a = a * b; return a; }
inline vec3i& operator*=(vec3i& a, const vec3i& b) { a = a * b; return a; }
inline vec4i& operator*=(vec4i& a, const vec4i& b) { a = a * b; return a; }
inline vec2i& operator/=(vec2i& a, const vec2i& b) { a = a / b; return a; }
inline vec3i& operator/=(vec3i& a, const vec3i& b) { a = a / b; return a; }
inline vec4i& operator/=(vec4i& a, const vec4i& b) { a = a / b; return a; }

// rgba

inline rgba operator+(const rgba& x, const rgba& y) { return { x.r + y.r, x.g + y.g, x.b + y.b, x.a + y.a }; }
inline rgba operator-(const rgba& x, const rgba& y) { return { x.r - y.r, x.g - y.g, x.b - y.b, x.a - y.a }; }
inline rgba operator*(const rgba& x, const rgba& y) { return { x.r * y.r, x.g * y.g, x.b * y.b, x.a * y.a }; }
inline rgba operator/(const rgba& x, const rgba& y) { return { x.r / y.r, x.g / y.g, x.b / y.b, x.a / y.a }; }

inline rgba operator+(const rgba& x) { return x; }
inline rgba operator-(const rgba& x) { return { -x.r, -x.g, -x.b, -x.a }; }

inline rgba& operator+=(rgba& x, const rgba& y) { x = x + y; return x; }
inline rgba& operator-=(rgba& x, const rgba& y) { x = x - y; return x; }
inline rgba& operator*=(rgba& x, const rgba& y) { x = x * y; return x; }
inline rgba& operator/=(rgba& x, const rgba& y) { x = x / y; return x; }

// etc

inline vec3 operator*(const mat33& a, const vec3& b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }
inline mat33 operator*(const mat33& a, const mat33& b) { return mat33(a * b.x, a * b.y, a * b.z); }

inline vec4 operator*(const mat44& a, const vec4& b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w); }
inline mat44 operator*(const mat44& a, const mat44& b) { return mat44(a * b.x, a * b.y, a * b.z, a * b.w); }

inline vec3 operator*(const transform& a, const vec3& b) { return { a.t + a.m * b }; }
inline transform operator*(const transform& a, const transform& b) { return { a.m * b.m, a.t + a.m * b.t }; }

inline bool operator==(vec2i& lhs, vec2i& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator!=(vec2i& lhs, vec2i& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }

inline vec2i ifloor(vec2 v) { return vec2i(ifloor(v.x), ifloor(v.y)); }
inline vec3i ifloor(vec3 v) { return vec3i(ifloor(v.x), ifloor(v.y), ifloor(v.z)); }
inline vec4i ifloor(vec4 v) { return vec4i(ifloor(v.x), ifloor(v.y), ifloor(v.z), ifloor(v.w)); }

inline vec2 to_vec2(vec2i v) { return vec2((f32)v.x, (f32)v.y); }
inline vec3 to_vec2(vec3i v) { return vec3((f32)v.x, (f32)v.y, (f32)v.z); }
inline vec4 to_vec2(vec4i v) { return vec4((f32)v.x, (f32)v.y, (f32)v.z, (f32)v.w); }

inline vec2 rotation(float a) { return vec2(cosf(a), sinf(a)); }
inline float rotation_of(const vec2& v) { return atan2f(v.y, v.x); }

inline vec2 min(const vec2& v0, const vec2& v1) { return vec2(min(v0.x, v1.x), min(v0.y, v1.y)); }
inline vec3 min(const vec3& v0, const vec3& v1) { return vec3(min(v0.x, v1.x), min(v0.y, v1.y), min(v0.z, v1.z)); }
inline vec4 min(const vec4& v0, const vec4& v1) { return vec4(min(v0.x, v1.x), min(v0.y, v1.y), min(v0.z, v1.z), min(v0.w, v1.w)); }

inline vec2 max(const vec2& v0, const vec2& v1) { return vec2(max(v0.x, v1.x), max(v0.y, v1.y)); }
inline vec3 max(const vec3& v0, const vec3& v1) { return vec3(max(v0.x, v1.x), max(v0.y, v1.y), max(v0.z, v1.z)); }
inline vec4 max(const vec4& v0, const vec4& v1) { return vec4(max(v0.x, v1.x), max(v0.y, v1.y), max(v0.z, v1.z), max(v0.w, v1.w)); }

inline float sum(const vec2& v) { return v.x + v.y; }
inline float sum(const vec3& v) { return v.x + v.y + v.z; }
inline float sum(const vec4& v) { return v.x + v.y + v.z + v.w; }

inline float dot(const vec2& v0, const vec2& v1) { return sum(v0 * v1); }
inline float dot(const vec3& v0, const vec3& v1) { return sum(v0 * v1); }
inline float dot(const vec4& v0, const vec4& v1) { return sum(v0 * v1); }

inline float length_sq(const vec2& v) { return sum(v * v); }
inline float length_sq(const vec3& v) { return sum(v * v); }
inline float length_sq(const vec4& v) { return sum(v * v); }

inline float length(const vec2& v) { return sqrtf(sum(v * v)); }
inline float length(const vec3& v) { return sqrtf(sum(v * v)); }
inline float length(const vec4& v) { return sqrtf(sum(v * v)); }

inline vec2 normalise(const vec2& v) { return v * vec2(1.0f / length(v)); }
inline vec3 normalise(const vec3& v) { return v * vec3(1.0f / length(v)); }
inline vec4 normalise(const vec4& v) { return v * vec4(1.0f / length(v)); }

inline vec2 reflect(const vec2& v, const vec2& norm) { return v - norm * (dot(v, norm) * 2.0f); }
inline vec3 reflect(const vec3& v, const vec3& norm) { return v - norm * (dot(v, norm) * 2.0f); }
inline vec4 reflect(const vec4& v, const vec4& norm) { return v - norm * (dot(v, norm) * 2.0f); }

inline vec2 project_onto(const vec2& v, const vec2& axis) { return axis * dot(v, axis); }
inline vec3 project_onto(const vec3& v, const vec3& axis) { return axis * dot(v, axis); }
inline vec4 project_onto(const vec4& v, const vec4& axis) { return axis * dot(v, axis); }

inline vec2 cancel(const vec2& v, const vec2& axis) { return v - axis * dot(v, axis); }
inline vec3 cancel(const vec3& v, const vec3& axis) { return v - axis * dot(v, axis); }
inline vec4 cancel(const vec4& v, const vec4& axis) { return v - axis * dot(v, axis); }

inline vec2 perp(const vec2& v) { return vec2(-v.y, v.x); }
inline float cross(const vec2& v0, const vec2& v1) { return v0.x * v1.y - v1.x * v0.y; }
inline vec3 cross(const vec3& v0, const vec3& v1) { return vec3(v0.y * v1.z - v1.y * v0.z, v0.z * v1.x - v1.z * v0.x, v0.x * v1.y - v1.x * v0.y); }

inline vec2 bevel_offset_from_normals(vec2 a, vec2 b) { return (perp(a) + a * cross(perp(b) - perp(a), b / cross(a, b))); }

mat44 perspective(float fov, float aspect, float z_near, float z_far);
mat44 perspective(float left, float right, float bottom, float top, float z_near, float z_far);
mat44 ortho(float left, float right, float bottom, float top, float z_near, float z_far);
vec3 unproject(vec3 projected, mat44 inv_proj_view, vec2 viewport);

mat33 transpose(const mat33& m);
mat44 transpose(const mat44& m);

mat33 inverse(const mat33& m);
mat44 inverse(const mat44& m);

mat33 scale3(const vec3& s);
mat33 rotate3_x(float theta);
mat33 rotate3_y(float theta);
mat33 rotate3_z(float theta);

transform scale(const vec3& s);
transform rotate_x(float theta);
transform rotate_y(float theta);
transform rotate_z(float theta);
transform translate(const vec3& v);

transform inverse(const transform& t);

transform look_at(const vec3& eye, const vec3& at, const vec3& up);
transform camera_look_at(const vec3& eye, const vec3& at, const vec3& up);

inline float luminance(const rgba& c) { return c.r * 0.299f + c.g * 0.587f + c.b * 0.114f; }
inline rgba saturation(const rgba& c, float v) { return lerp(rgba(luminance(c), c.a), c, v); }
inline rgba lighten(const rgba& c, float v) { return lerp(c, rgba(1.0f, c.a), v); }
inline rgba darken(const rgba& c, float v) { return lerp(c, rgba(0.0f, c.a), v); }

inline bool overlaps_x(const aabb2& a, const aabb2& b)	{ return a.max.x > b.min.x && a.min.x < b.max.x; }
inline bool overlaps_y(const aabb2& a, const aabb2& b)	{ return a.max.y > b.min.y && a.min.y < b.max.y; }

inline bool overlaps(const aabb2& a, const aabb2& b)	{ return overlaps_x(a, b) && overlaps_y(a, b); }
inline bool contains(const aabb2& a, const vec2& p)		{ return (a.min.x <= p.x && p.x <= a.max.x) && (a.min.y <= p.y && p.y <= a.max.y); }

inline aabb2 offset(const aabb2& a, vec2 offset_by)		{ return aabb2(a.min + offset_by, a.max + offset_by); }
inline aabb2 include(const aabb2& a, vec2 p)			{ return aabb2(min(a.min, p), max(a.max, p)); }
inline aabb2 inflate(const aabb2& a, vec2 inflate_by)	{ return aabb2(a.min - inflate_by, a.max + inflate_by); }
inline aabb2 enclosing(const aabb2& a, const aabb2& b)	{ return aabb2(min(a.min, b.min), max(a.max, b.max)); }

aabb2 offset_enclosing(const aabb2& a, vec2 d);

frustum make_frustum(const mat44& m);
vec3 intersect_planes_3(const vec4& a, const vec4& b, const vec4& c);

mat44 top_down_proj_view(vec2 centre, float fov, float aspect, float virtual_height, float z_near, float z_far);
mat44 fit_ui_proj_view(float virtual_width, float virtual_height, float aspect, float z_near, float z_far);