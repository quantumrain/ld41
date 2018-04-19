#include "pch.h"
#include "platform.h"
#include "base.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_PNG
#include "stb_image.h"

extern HWND g_win_hwnd;
extern const wchar_t* g_win_name;

// debug

void debug(const char* txt, ...) {
	char buf[512];

	va_list ap;

	va_start(ap, txt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, txt, ap);
	va_end(ap);

	OutputDebugStringA(buf);
	OutputDebugStringA("\r\n");
}

void panic(const char* txt, ...) {
	char buf[512];

	va_list ap;

	va_start(ap, txt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, txt, ap);
	va_end(ap);

	char title[256];
	WideCharToMultiByte(CP_UTF8, 0, g_win_name, -1, title, 256, 0, FALSE);

	MessageBoxA(g_win_hwnd, buf, title, MB_ICONERROR | MB_OK);
	ExitProcess(0);
}

// file

bool load_file(file_buf* fb, const char* path) {
	fb->destroy();

	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;

	DWORD size = GetFileSize(h, 0);

	if (size < (64 * 1024 * 1024)) {
		if ((fb->data = new u8[size]) != 0) {
			fb->size = (int)size;
			DWORD bytes = 0;
			if (!ReadFile(h, fb->data, size, &bytes, 0) || (bytes != size)) {
				fb->destroy();
			}
		}
	}

	CloseHandle(h);

	return fb->data != 0;
}

u8* image_from_memory(u8* data, int size, int* width, int* height) {
	return stbi_load_from_memory(data, size, width, height, 0, 4);
}

// timer

u64 timer_ticks() {
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart;
}

u64 timer_freq() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

double timer_ticks_to_ms(u64 ticks) {
	return ((double)ticks * 1000.0) / (double)timer_freq();
}

// random

void random::set_seed(u64 seed) { // based on splitmix64
	u64 x = (seed ^ 0xC6A4A7935BD1E995ull) * 0xC6A4A7935BD1E995ull;

	{
		u64 z = x += 0x9E3779B97F4A7C15ull;
		z  = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z  = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		_a = (z ^ (z >> 31));
	}
	{
		u64 z = x += 0x9E3779B97F4A7C15ull;
		z  = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z  = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		_b = (z ^ (z >> 31));
	}
}

u64 random::rand64() { // based on xorshift128+
	u64 a = _a;
	u64 b = _b;

	a ^= a << 23;
	a = a ^ b ^ (a >> 18) ^ (b >> 5);

	_a = b;
	_b = a;

	return a + b;
}

double random::frand64()								{ return (rand64() >> 11) * (1.0 / 9007199254740992.0); }

bool random::chance(int n, int out_of)					{ return (rand64() % out_of) < n; }

int random::rand(int max)								{ return (max > 0) ? (rand64() % max) : 0; }
int random::range(int min, int max)						{ return (min < max) ? (min + (rand64() % (max - min + 1))) : min; }
int random::range(int size)								{ return range(-size, size); }

float random::rand(float max)							{ return (float)frand64() * max; }
float random::range(float min, float max)				{ return (float)(min + frand64() * (max - min)); }
float random::range(float size)							{ return (float)(frand64() * size * 2.0f - size);	}

vec2 random::rand(const vec2& max)						{ return vec2(rand(max.x), rand(max.y)); }
vec2 random::range(const vec2& min, const vec2& max)	{ return vec2(range(min.x, max.x), range(min.y, max.y)); }
vec2 random::range(const vec2& size)					{ return vec2(range(size.x), range(size.y)); }

vec3 random::rand(const vec3& max)						{ return vec3(rand(max.x), rand(max.y), rand(max.z)); }
vec3 random::range(const vec3& min, const vec3& max)	{ return vec3(range(min.x, max.x), range(min.y, max.y), range(min.z, max.z)); }
vec3 random::range(const vec3& size)					{ return vec3(range(size.x), range(size.y), range(size.z)); }

vec4 random::rand(const vec4& max)						{ return vec4(rand(max.x), rand(max.y), rand(max.z), rand(max.w)); }
vec4 random::range(const vec4& min, const vec4& max)	{ return vec4(range(min.x, max.x), range(min.y, max.y), range(min.z, max.z), range(min.w, max.w)); }
vec4 random::range(const vec4& size)					{ return vec4(range(size.x), range(size.y), range(size.z), range(size.w)); }

vec2i random::rand(const vec2i& max)					{ return vec2i(rand(max.x), rand(max.y)); }
vec2i random::range(const vec2i& min, const vec2i& max)	{ return vec2i(range(min.x, max.x), range(min.y, max.y)); }
vec2i random::range(const vec2i& size)					{ return vec2i(range(size.x), range(size.y)); }

vec3i random::rand(const vec3i& max)					{ return vec3i(rand(max.x), rand(max.y), rand(max.z)); }
vec3i random::range(const vec3i& min, const vec3i& max)	{ return vec3i(range(min.x, max.x), range(min.y, max.y), range(min.z, max.z)); }
vec3i random::range(const vec3i& size)					{ return vec3i(range(size.x), range(size.y), range(size.z)); }

vec4i random::rand(const vec4i& max)					{ return vec4i(rand(max.x), rand(max.y), rand(max.z), rand(max.w)); }
vec4i random::range(const vec4i& min, const vec4i& max)	{ return vec4i(range(min.x, max.x), range(min.y, max.y), range(min.z, max.z), range(min.w, max.w)); }
vec4i random::range(const vec4i& size)					{ return vec4i(range(size.x), range(size.y), range(size.z), range(size.w)); }

// string

void sz_copy(char* d, const char* s, int max) {
	if (max > 0) {
		for(; (max > 1) && *s; max--)
			*d++ = *s++;
		*d = '\0';
	}
}

void sz_copy(char* d, const char* s0, const char* s1, int max) {
	if (max > 0) {
		for(; (max > 1) && (s0 < s1); max--)
			*d++ = *s0++;

		*d = '\0';
	}
}

const char* sz_find_last_or_end(const char* p, char c) {
	for(; *p != c; p++) {
		if (*p == '\0') return p;
	}

	for(;;) {
		for(const char* e = p++; *p != c; p++) {
			if (*p == '\0') return e;
		}
	}
}

// hash

u64 murmur_hash_64(const void* key, u32 len, u64 seed) {
	u64 m = 0xC6A4A7935BD1E995ull;
	u64 h = seed ^ (len * m);

	const u64* data = (const u64*)key;
	const u64* end = data + (len / 8);

	while(data != end) {
		u64 k = *data++; // TODO: Endian

		k *= m; 
		k ^= k >> 47; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char* data2 = (const unsigned char*)data;

	switch(len & 7) {
		case 7: h ^= u64(data2[6]) << 48;
		case 6: h ^= u64(data2[5]) << 40;
		case 5: h ^= u64(data2[4]) << 32;
		case 4: h ^= u64(data2[3]) << 24;
		case 3: h ^= u64(data2[2]) << 16;
		case 2: h ^= u64(data2[1]) << 8;
		case 1: h ^= u64(data2[0]);
				h *= m;
	};

	h ^= h >> 47;
	h *= m;
	h ^= h >> 47;

	return h;
}

u64 murmur_hash_64(u64 key, u64 seed) {
	u64 m = 0xC6A4A7935BD1E995ull;
	u64 h = seed ^ (sizeof(key) * m);

	key *= m;
	key ^= key >> 47;
	key *= m;

	h ^= key;
	h *= m;

	h ^= h >> 47;
	h *= m;
	h ^= h >> 47;

	return h;
}
