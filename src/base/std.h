#pragma once

// debug

void debug(const char* txt, ...);
__declspec(noreturn) void panic(const char* txt, ...);

// file

struct file_buf {
	u8* data;
	int size;

	file_buf() : data(), size() { }
	~file_buf() { destroy(); }

	void destroy() { delete [] data; data = 0; size = 0; }
};

bool load_file(file_buf* fb, const char* path);
u8* image_from_memory(u8* data, int size, int* width, int* height);

// timer

u64 timer_ticks();
u64 timer_freq();
double timer_ticks_to_ms(u64 ticks);

// random

struct random {
	random() { set_seed(0); }
	explicit random(u64 seed) { set_seed(seed); }

	random gen() { return random(rand64()); }

	void set_seed(u64 seed);

	u64 rand64();
	double frand64();

	bool chance(int n, int out_of);

	int rand(int max);
	int range(int min, int max);
	int range(int size);

	float rand(float max);
	float range(float min, float max);
	float range(float size);

	vec2 rand(const vec2& max);
	vec2 range(const vec2& min, const vec2& max);
	vec2 range(const vec2& size);

	vec3 rand(const vec3& max);
	vec3 range(const vec3& min, const vec3& max);
	vec3 range(const vec3& size);

	vec4 rand(const vec4& max);
	vec4 range(const vec4& min, const vec4& max);
	vec4 range(const vec4& size);

	vec2i rand(const vec2i& max);
	vec2i range(const vec2i& min, const vec2i& max);
	vec2i range(const vec2i& size);

	vec3i rand(const vec3i& max);
	vec3i range(const vec3i& min, const vec3i& max);
	vec3i range(const vec3i& size);

	vec4i rand(const vec4i& max);
	vec4i range(const vec4i& min, const vec4i& max);
	vec4i range(const vec4i& size);

	u64 _a, _b;
};

void sz_copy(char* d, const char* s, int max);
void sz_copy(char* d, const char* s0, const char* s1, int max);
const char* sz_find_last_or_end(const char* p, char c);

u64 murmur_hash_64(const void* key, u32 len, u64 seed);
u64 murmur_hash_64(u64 key, u64 seed);