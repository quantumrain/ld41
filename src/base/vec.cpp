#include "pch.h"
#include "base.h"

mat44 perspective(float fov, float aspect, float z_near, float z_far) {
	float top	= tanf(fov * 0.5f) * z_near;
	float right	= top * aspect;

	return perspective(-right, right, -top, top, z_near, z_far);
}

mat44 perspective(float left, float right, float bottom, float top, float z_near, float z_far) {
	float a = (right + left) / (right - left);
	float b = (top + bottom) / (top - bottom);
	float c = z_far / (z_near - z_far);
	float d = (z_near * z_far) / (z_near - z_far);

	float xx = (2.0f * z_near) / (right - left);
	float yy = (2.0f * z_near) / (top - bottom);

	return mat44(
			vec4(xx,	0.0f,	a,		0.0f),
			vec4(0.0f,	yy,		b,		0.0f),
			vec4(0.0f,	0.0f,	c,		-1.0f),
			vec4(0.0f,	0.0f,	d,		0.0f)
		);
}

mat44 ortho(float left, float right, float bottom, float top, float z_near, float z_far) {
	float tx = (right + left) / (left - right);
	float ty = (top + bottom) / (bottom - top);
	float tz = z_near / (z_near - z_far);
		
	float xx = 2.0f / (right - left);
	float yy = 2.0f / (top - bottom);
	float zz = 1.0f / (z_near - z_far);

	return mat44(
			vec4(xx,	0.0f,	0.0f,	0.0f),
			vec4(0.0f,	yy,		0.0f,	0.0f),
			vec4(0.0f,	0.0f,	zz,		0.0f),
			vec4(tx,	ty,		tz,		1.0f)
		);
}

vec3 unproject(vec3 projected, mat44 inv_proj_view, vec2 viewport) {
	float mx = projected.x / viewport.x;
	float my = projected.y / viewport.y;

	mx = (mx * 2.0f) - 1.0f;
	my = -((my * 2.0f) - 1.0f);

	vec4 p = inv_proj_view.x * mx + inv_proj_view.y * my + inv_proj_view.w;
	vec4 q = p + inv_proj_view.z;

	p /= p.w;
	q /= q.w;

	vec3 plane_p { 0.0f, 0.0f, projected.z };
	vec3 plane_d { 0.0f, 0.0f, 1.0f };

	vec3 line_d	= (vec3)p - (vec3)q;
	vec3 pos	= (vec3)p + line_d * dot(plane_p - (vec3)p, plane_d) / dot(line_d, plane_d);

	return vec3(pos.x, pos.y, projected.z);
}

mat33 transpose(const mat33& m) {
	return mat33(
			m.x.x, m.y.x, m.z.x,
			m.x.y, m.y.y, m.z.y,
			m.x.z, m.y.z, m.z.z
		);
}

mat44 transpose(const mat44& m) {
	return mat44(
			m.x.x, m.y.x, m.z.x, m.w.x,
			m.x.y, m.y.y, m.z.y, m.w.y,
			m.x.z, m.y.z, m.z.z, m.w.z,
			m.x.w, m.y.w, m.z.w, m.w.w
		);
}

mat33 inverse(const mat33& m) {
	float i = 1.0f / (m.x.x * (m.y.y * m.z.z - m.z.y * m.y.z) - m.y.x * (m.x.y * m.z.z - m.z.y * m.x.z) + m.z.x * (m.x.y * m.y.z - m.y.y * m.x.z));

	return mat33(
			+(m.y.y * m.z.z - m.z.y * m.y.z) * i, -(m.x.y * m.z.z - m.z.y * m.x.z) * i, +(m.x.y * m.y.z - m.y.y * m.x.z) * i,
			-(m.y.x * m.z.z - m.z.x * m.y.z) * i, +(m.x.x * m.z.z - m.z.x * m.x.z) * i, -(m.x.x * m.y.z - m.y.x * m.x.z) * i,
			+(m.y.x * m.z.y - m.z.x * m.y.y) * i, -(m.x.x * m.z.y - m.z.x * m.x.y) * i, +(m.x.x * m.y.y - m.y.x * m.x.y) * i
		);
}

mat44 inverse(const mat44& m) {
	float c0 = m.x.x * m.y.y - m.x.y * m.y.x;
	float c1 = m.x.x * m.y.z - m.x.z * m.y.x;
	float c2 = m.x.x * m.y.w - m.x.w * m.y.x;
	float c3 = m.x.y * m.y.z - m.x.z * m.y.y;
	float c4 = m.x.y * m.y.w - m.x.w * m.y.y;
	float c5 = m.x.z * m.y.w - m.x.w * m.y.z;
	float d0 = m.z.x * m.w.y - m.z.y * m.w.x;
	float d1 = m.z.x * m.w.z - m.z.z * m.w.x;
	float d2 = m.z.x * m.w.w - m.z.w * m.w.x;
	float d3 = m.z.y * m.w.z - m.z.z * m.w.y;
	float d4 = m.z.y * m.w.w - m.z.w * m.w.y;
	float d5 = m.z.z * m.w.w - m.z.w * m.w.z;

	float i = 1.0f / (+ c0 * d5 - c1 * d4 + c2 * d3 + c3 * d2 - c4 * d1 + c5 * d0);

	return mat44(
			(+ m.y.y * d5 - m.y.z * d4 + m.y.w * d3) * i, (- m.x.y * d5 + m.x.z * d4 - m.x.w * d3) * i, (+ m.w.y * c5 - m.w.z * c4 + m.w.w * c3) * i, (- m.z.y * c5 + m.z.z * c4 - m.z.w * c3) * i,
			(- m.y.x * d5 + m.y.z * d2 - m.y.w * d1) * i, (+ m.x.x * d5 - m.x.z * d2 + m.x.w * d1) * i, (- m.w.x * c5 + m.w.z * c2 - m.w.w * c1) * i, (+ m.z.x * c5 - m.z.z * c2 + m.z.w * c1) * i,
			(+ m.y.x * d4 - m.y.y * d2 + m.y.w * d0) * i, (- m.x.x * d4 + m.x.y * d2 - m.x.w * d0) * i, (+ m.w.x * c4 - m.w.y * c2 + m.w.w * c0) * i, (- m.z.x * c4 + m.z.y * c2 - m.z.w * c0) * i,
			(- m.y.x * d3 + m.y.y * d1 - m.y.z * d0) * i, (+ m.x.x * d3 - m.x.y * d1 + m.x.z * d0) * i, (- m.w.x * c3 + m.w.y * c1 - m.w.z * c0) * i, (+ m.z.x * c3 - m.z.y * c1 + m.z.z * c0) * i
		);
}

mat33 scale3(const vec3& s) {
	return mat33(
			s.x,	0.0f,	0.0f,
			0.0f,	s.y,	0.0f,
			0.0f,	0.0f,	s.z
		);
}

mat33 rotate3_x(float theta) {
	float c = cosf(theta);
	float s = sinf(theta);

	return mat33(
			1.0f,	0.0f,	0.0f,
			0.0f,	c,		s,
			0.0f,	-s,		c
		);
}

mat33 rotate3_y(float theta) {
	float c = cosf(theta);
	float s = sinf(theta);

	return mat33(
			c,		0.0f,	-s,
			0.0f,	1.0f,	0.0f,
			s,		0.0f,	c
		);
}

mat33 rotate3_z(float theta) {
	float c = cosf(theta);
	float s = sinf(theta);

	return mat33(
			c,		s,		0.0f,
			-s,		c,		0.0f,
			0.0f,	0.0f,	1.0f
		);
}

transform scale(const vec3& s)		{ return { scale3(s), {} }; }
transform rotate_x(float theta)		{ return { rotate3_x(theta), {} }; }
transform rotate_y(float theta)		{ return { rotate3_y(theta), {} }; }
transform rotate_z(float theta)		{ return { rotate3_z(theta), {} }; }
transform translate(const vec3& v)	{ return { {}, v }; }

transform inverse(const transform& t) {
	mat33 i = inverse(t.m);
	return { i, i * -t.t };
}

transform look_at(const vec3& eye, const vec3& at, const vec3& up) {
	vec3 z = normalise(eye - at);
	vec3 x = normalise(cross(up, z));
	vec3 y = cross(z, x);

	return { { x, y, z }, eye };
}

transform camera_look_at(const vec3& eye, const vec3& at, const vec3& up) {
	vec3 z = normalise(at - eye);
	vec3 x = normalise(cross(up, z));
	vec3 y = cross(z, x);

	return { { x.x, y.x, z.x, x.y, y.y, z.y, x.z, y.z, z.z }, { dot(x, eye), dot(y, eye), dot(z, eye) } };
}

frustum make_frustum(const mat44& m) {
	frustum f;

	f.plane[frustum::LEFT]   = { m.x.w + m.x.x, m.y.w + m.y.x, m.z.w + m.z.x, m.w.w + m.w.x };
	f.plane[frustum::RIGHT]  = { m.x.w - m.x.x, m.y.w - m.y.x, m.z.w - m.z.x, m.w.w - m.w.x };
	f.plane[frustum::TOP]    = { m.x.w - m.x.y, m.y.w - m.y.y, m.z.w - m.z.y, m.w.w - m.w.y };
	f.plane[frustum::BOTTOM] = { m.x.w + m.x.y, m.y.w + m.y.y, m.z.w + m.z.y, m.w.w + m.w.y };
	f.plane[frustum::NEAR]   = { m.x.z,         m.y.z,         m.z.z,         m.w.z         };
	f.plane[frustum::FAR]    = { m.x.w - m.x.z, m.y.w - m.y.z, m.z.w - m.z.z, m.w.w - m.w.z };

	return f;
}

vec3 intersect_planes_3(const vec4& a, const vec4& b, const vec4& c) {
	vec3 ab = cross((vec3)a, (vec3)b);
	vec3 bc = cross((vec3)b, (vec3)c);
	vec3 ca = cross((vec3)c, (vec3)a);
	return (-a.w * bc - b.w * ca - c.w * ab) / dot((vec3)a, bc);
}

mat44 top_down_proj_view(vec2 centre, float fov, float aspect, float virtual_height, float z_near, float z_far) {
	float camera_height	= virtual_height / (tanf(deg_to_rad(fov) * 0.5f) * 2.0f);
	mat44 proj			= perspective(deg_to_rad(fov), aspect, z_near, z_far);
	mat44 view			= (mat44)camera_look_at(vec3(centre, camera_height), vec3(centre, 0.0f), vec3(0, -1, 0));
	return proj * view;
}

mat44 fit_ui_proj_view(float virtual_width, float virtual_height, float aspect, float z_near, float z_far) {
	float width  = virtual_width;
	float height = virtual_height;

	if (aspect < (width / height))
		height = width / aspect;
	else
		width = height * aspect;

	float left = (virtual_width - width) * 0.5f;
	float top  = (virtual_height - height) * 0.5f;

	return ortho(left, left + width, top + height, top, z_near, z_far);
}

aabb2 offset_enclosing(const aabb2& a, vec2 d) {
	vec2 min = a.min;
	vec2 max = a.max;

	if (d.x < 0.0f) min.x += d.x; else max.x += d.x;
	if (d.y < 0.0f) min.y += d.y; else max.y += d.y;

	return aabb2(min, max);
}