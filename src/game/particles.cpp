#include "pch.h"
#include "game.h"

random g_rand;

struct particle {
	vec2 pos;
	vec2 vel;
	float damp;
	float size0;
	float size1;
	float rot;
	float rot_v;
	rgba c;
	int time;
	int lifetime;
};

struct psys {
	array<particle> particles;
};

psys g_particles;

void psys_init(int max_particles) {
	g_particles.particles.reserve(max_particles);
}

void psys_update() {
	psys* sys = &g_particles;

	for(int i = 0; i < sys->particles.size(); i++) {
		particle* p = &sys->particles[i];

		if (p->time++ >= p->lifetime) {
			sys->particles.swap_remove(i);
			i--;
			continue;
		}

		p->pos += p->vel * DT;
		p->vel *= p->damp;
	}
}

void psys_render(draw_context* dc) {
	psys* sys = &g_particles;

	for(auto& p : sys->particles) {
		float f = p.time / (float)p.lifetime;
		float s = lerp(p.size0, p.size1, f);

		dc->shape_outline(p.pos, 3, s, p.rot, 0.5f, p.c * rgba(1.0f - f, 0.0f));
	}
}

void psys_spawn(vec2 pos, vec2 vel, float damp, float size0, float size1, float rot_v, rgba c, int lifetime) {
	psys* sys = &g_particles;

	if (sys->particles.size() >= sys->particles.capacity())
		return;

	particle* p = sys->particles.push_back(particle());

	p->pos = pos;
	p->vel = vel;
	p->damp = damp;
	p->size0 = size0;
	p->size1 = size1;
	p->rot = g_rand.range(PI);
	p->rot_v = rot_v;
	p->c = c;
	p->time = 0;
	p->lifetime = lifetime;
}

// effects

void fx_explosion(vec2 pos, float strength, int count, rgba c, float psize) {
	int num_a = count * 3;
	int num_b = count * 2;

	for(int i = 0; i < num_a; i++) {
		vec2 v = rotation(g_rand.range(PI)) * square(0.5f + g_rand.rand(1.0f)) * 200.0f * strength;

		psys_spawn(pos, v, 0.9f, g_rand.range(6.0f, 10.0f) * psize, 1.0f * psize, g_rand.range(1.0f), c * 1.5f, g_rand.range(15, 20));
	}

	for(int i = 0; i < num_b; i++) {
		vec2 v = rotation(g_rand.range(PI)) * square(0.5f + g_rand.rand(1.0f)) * 350.0f * strength;

		psys_spawn(pos, v, 0.9f, 2.0f * psize, 2.0f * psize,  g_rand.range(1.0f), c * 2.5f, g_rand.range(8, 12));
	}
}

