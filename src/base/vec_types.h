#pragma once

// vec*

template<typename T> struct vec2t;
template<typename T> struct vec3t;
template<typename T> struct vec4t;

template<typename T> struct vec2t {
	T x, y;

	vec2t()           : x(),   y()   { }
	vec2t(T xy)       : x(xy), y(xy) { }
	vec2t(T x_, T y_) : x(x_), y(y_) { }

	T& operator[](int i) { assert(i >= 0 && i < 2); return (&x)[i]; }
};

template<typename T> struct vec3t {
	T x, y, z;

	vec3t()                         : x(),     y(),     z()    { }
	vec3t(T xyz)                    : x(xyz),  y(xyz),  z(xyz) { }
	vec3t(T x_, T y_, T z_)         : x(x_),   y(y_),   z(z_)  { }
	vec3t(const vec2t<T>& xy, T z_) : x(xy.x), y(xy.y), z(z_)  { }

	explicit operator vec2t<T>() const { return vec2t<T>(x, y); }

	T& operator[](int i) { assert(i >= 0 && i < 3); return (&x)[i]; }
};

template<typename T> struct vec4t {
	T x, y, z, w;

	vec4t()                                       : x(),      y(),      z(),      w()     { }
	vec4t(T xyzw)                                 : x(xyzw),  y(xyzw),  z(xyzw),  w(xyzw) { }
	vec4t(T x_, T y_, T z_, T w_)                 : x(x_),    y(y_),    z(z_),    w(w_)   { }
	vec4t(const vec2t<T>& xy, T z_, T w_)         : x(xy.x),  y(xy.y),  z(z_),    w(w_)   { }
	vec4t(const vec2t<T>& xy, const vec2t<T>& zw) : x(xy.x),  y(xy.y),  z(zw.x),  w(zw.y) { }
	vec4t(const vec3t<T>& xyz, T w_)              : x(xyz.x), y(xyz.y), z(xyz.z), w(w_)   { }

	explicit operator vec2t<T>() const { return vec2t<T>(x, y); }
	explicit operator vec3t<T>() const { return vec3t<T>(x, y, z); }

	T& operator[](int i) { assert(i >= 0 && i < 4); return (&x)[i]; }
};

using vec2 = vec2t<f32>;
using vec3 = vec3t<f32>;
using vec4 = vec4t<f32>;

using vec2i = vec2t<i32>;
using vec3i = vec3t<i32>;
using vec4i = vec4t<i32>;

// mat*

struct mat33 {
	vec3 x, y, z;

	mat33() : x(1.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f), z(0.0f, 0.0f, 1.0f) { }
	mat33(const vec3& x_, const vec3& y_, const vec3& z_) : x(x_), y(y_), z(z_) { }
	mat33(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22) : x(m00, m01, m02), y(m10, m11, m12), z(m20, m21, m22) { }
};

struct mat44 {
	vec4 x, y, z, w;

	mat44() : x(1.0f, 0.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f, 0.0f), z(0.0f, 0.0f, 1.0f, 0.0f), w(0.0f, 0.0f, 0.0f, 1.0f) { }
	mat44(const vec4& x_, const vec4& y_, const vec4& z_, const vec4& w_) : x(x_), y(y_), z(z_), w(w_) { }

	mat44(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33
		) : x(m00, m01, m02, m03), y(m10, m11, m12, m13), z(m20, m21, m22, m23), w(m30, m31, m32, m33)
	{ }
};

// transform

struct transform {
	mat33 m;
	vec3 t;

	explicit operator mat44() const { return mat44(vec4(m.x, 0.0f), vec4(m.y, 0.0f), vec4(m.z, 0.0f), vec4(t, 1.0f)); }
};

// rgba

enum class colours : u32 { // clrs.cc
	NAVY   = 0x001F3F, BLUE  = 0x0074D9, AQUA   = 0x7FDBFF, TEAL    = 0x39CCCC,
	OLIVE  = 0x3D9970, GREEN = 0x2ECC40, LIME   = 0x01FF70, YELLOW  = 0xFFDC00,
	ORANGE = 0xFF851B, RED   = 0xFF4136, MAROON = 0x85144B, FUCHSIA = 0xF012BE,
	PURPLE = 0xB10DC9, BLACK = 0x111111, GREY   = 0xAAAAAA, SILVER  = 0xDDDDDD
};

#define ARGB_A(rgba_) ((((u32)(rgba_)) >> 24) & 0xFF)
#define ARGB_R(rgba_) ((((u32)(rgba_)) >> 16) & 0xFF)
#define ARGB_G(rgba_) ((((u32)(rgba_)) >>  8) & 0xFF)
#define ARGB_B(rgba_) ((((u32)(rgba_))      ) & 0xFF)

struct rgba {
	float r, g, b, a;

	rgba(f32 r_, f32 g_, f32 b_, f32 a_) : r(r_), g(g_), b(b_), a(a_) { }
	rgba()                          : rgba(1.0f,  1.0f,  1.0f,  1.0f ) { }
	rgba(f32 rgba_)                 : rgba(rgba_, rgba_, rgba_, rgba_) { }
	rgba(f32 rgb_, f32 a_)          : rgba(rgb_,  rgb_,  rgb_,  a_   ) { }
	rgba(vec4 v_)                   : rgba(v_.x,  v_.y,  v_.z,  v_.w ) { }
	rgba(colours c_, f32 a_ = 1.0f) : rgba(ARGB_R(c_) / 255.0f, ARGB_G(c_) / 255.0f, ARGB_B(c_) / 255.0f, a_) { }
	explicit rgba(u32 argb)			: rgba(ARGB_R(argb) / 255.0f, ARGB_G(argb) / 255.0f, ARGB_B(argb) / 255.0f, ARGB_A(argb) / 255.0f) { }
	explicit rgba(i32 argb)			: rgba((u32)argb) { }
};

// etc

struct frustum {
	vec4 plane[6];
	enum side { LEFT, RIGHT, TOP, BOTTOM, NEAR, FAR };
	const vec4& operator[](side s) const { assert(s >= LEFT && s <= FAR); return plane[s]; }
};

struct aabb2 {
	vec2 min, max;

	aabb2() = default;
	aabb2(vec2 min_, vec2 max_) : min(min_), max(max_) { }
};