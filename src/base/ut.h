#pragma once

#define NOCOPY(type) type(const type&) = delete; void operator=(const type&) = delete;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

const float PI		= 3.1415926536f;
const float TAU		= 6.2831853072f;
const float INV_PI	= 0.3183098862f;
const float INV_TAU	= 0.1591549431f;

struct placement_new { };

inline void* operator new(size_t, const placement_new&, void* where) { return where; }
inline void operator delete(void*, const placement_new&, void*) { }

template<typename T> T* construct(T* p)											{ return new(placement_new(), (void*)p) T(); }
template<typename T, typename Src> T* copy_construct(T* p, const Src& src)		{ return new(placement_new(), p) T(src); }
template<typename T> void construct_range(T* first, T* last)					{ for(; first != last; ++first) construct(first); }
template<typename T> void destruct(T* p)										{ p->~T(); }
template<typename T> void destruct_range(T* first, T* last)						{ for(; first != last; ++first) destruct(first); }

template<typename T> T min(const T& a, const T& b)								{ return a < b ? a : b; }
template<typename T> T max(const T& a, const T& b)								{ return a < b ? b : a; }
template<typename T> T clamp(const T& v, const T& low, const T& high)			{ return (v < low) ? low : ((v > high) ? high : v); }
template<typename T> T saturate(const T& v)			                            { return clamp(v, T{0}, T{1}); }
template<typename T> void swap(T& a, T& b)										{ T t = b; b = a; a = t; }
template<typename T> T lerp(const T& a, const T& b, float t)					{ return a + (b - a) * t; }

inline float square(float v)													{ return v * v; }
inline float square_signed(float v)												{ return v * fabsf(v); }
inline int ifloor(float f)														{ int i = (int)f; return f < i ? i - 1 : i; }
inline int iceil(float f)														{ int i = (int)f; return f > i ? i + 1 : i; }
inline int isign(float v)														{ return v < 0.0f ? -1 : (v > 0.0f ? 1 : 0); }
inline float pow3(float v)														{ return v * v * v; }

inline float rad_to_deg(float x) { return x * (180.0f / PI); }
inline float deg_to_rad(float x) { return x * (PI / 180.0f); }
inline float normalise_radians(float a) { return fmodf(fmodf(a + PI, TAU) + TAU, TAU) - PI; }
inline float normalise_degrees(float a) { return fmodf(fmodf(a + 180.0f, 360.0f) + 360.0f, 360.0f) - 180.0f; }
inline float gaussian(float n, float theta) { return ((1.0f / sqrtf(2.0f * 3.14159f * theta)) * expf(-(n * n) / (2.0f * theta * theta))); }

template<typename T>
struct array {
	array() : _elems(), _size(), _cap() { }
	~array() { free(); }

	bool reserve(int min_cap) {
		if (min_cap <= _cap) return true;

		T* elems = (T*)malloc(sizeof(T) * min_cap);

		if (!elems) return false;

		memcpy(elems, _elems, sizeof(T) * _size);
		::free((void*)_elems);

		_elems	= elems;
		_cap	= min_cap;

		return true;
	}

	bool ensure_available(int min_available) {
		if (min_available <= _cap - _size) return true;
		return reserve(max(_size + min_available, max(_cap * 2, 64 / (int)sizeof(T))));
	}

	bool set_size(int size) {
		if (!reserve(size))
			return false;

		if (size < _size) {
			destruct_range(_elems + size, _elems + _size);
			_size = size;
		}
		else if (size > _size) {
			construct_range(_elems + _size, _elems + size);
			_size = size;
		}

		return true;
	}

	void free() {
		destruct_range(_elems, _elems + _size);
		::free(_elems);
		abandon();
	}

	void remove_all() {
		destruct_range(_elems, _elems + _size);
		_size = 0;
	}

	void abandon() {
		_elems = 0; _size = 0; _cap = 0;
	}

	T* push_back(const T& v) {
		if (!ensure_available(1)) return 0;
		return copy_construct(_elems + _size++, v);
	}

	T* push_back() {
		if (!ensure_available(1)) return 0;
		return construct(_elems + _size++);
	}

	T* acquire_spare() {
		return (_size < _cap) ? construct(_elems + _size++) : 0;
	}

	void pop_back() {
		if (_size > 0) destruct(_elems + (--_size));
	}

	T* insert(int i, const T& v) {
		if (i >= 0 && i <= _size) {
			if (ensure_available(1)) {
				T* p = _elems + i;
				memmove(p + 1, p, sizeof(T) * (_size - i));
				_size++;
				return copy_construct(p, v);
			}
		}
		return 0;
	}

	void remove(int i) {
		if (i >= 0 && i < _size) {
			destruct(_elems + i);
			memmove(_elems + i, _elems + i + 1, sizeof(T) * (_size - 1));
			_size--;
		}
	}

	void swap_remove(int i) {
		if (i >= 0 && i < _size) {
			destruct(_elems + i);
			_size--;
			if (i != _size) memcpy(_elems + i, _elems + _size, sizeof(T));
		}
	}

	bool empty() const { return _size == 0; }
	bool not_empty() const { return _size != 0; }

	int size() const { return _size; }
	int capacity() const { return _cap; }

	T* at(int i) { return (i >= 0 && i < _size) ? _elems + i : 0; }
	T& operator[](int i) { assert(i >= 0 && i < _size); return _elems[i]; }

	T& front() { assert(_size > 0); return _elems[0]; }
	T& back() { assert(_size > 0); return _elems[_size - 1]; }

	T* begin() { return _elems; }
	T* end() { return _elems + _size; }

	T* _elems;
	int _size;
	int _cap;

	NOCOPY(array<T>);
};

template<typename T>
struct list {
	list() { }
	~list() { free(); }

	bool reserve(int min_cap) { return _arr.reserve(min_cap); }
	bool ensure_available(int min_available) { return _arr.ensure_available(min_available); }

	void free() {
		for(T* p : _arr) delete p;
		_arr.free();
	}

	T* push_back(T* p) {
		if (p && _arr.push_back(p)) return p;
		delete p;
		return 0;
	}

	void pop_back() {
		if (_arr.not_empty()) {
			delete _arr.back();
			_arr.pop_back();
		}
	}

	T* insert(int i, T* p) {
		if (p && _arr.insert(i, p)) return p;
		delete p;
		return 0;
	}

	void remove(int i) {
		if (i >= 0 && i < _arr.size()) {
			delete _arr[i];
			_arr.remove(i);
		}
	}

	void swap_remove(int i) {
		if (i >= 0 && i < _arr.size()) {
			delete _arr[i];
			_arr.swap_remove(i);
		}
	}

	void remove_all() {
		for(T* p : _arr) delete p;
		_arr.remove_all();
	}

	bool empty() const { return _arr.empty(); }
	bool not_empty() const { return _arr.not_empty(); }

	int size() const { return _arr.size(); }
	int capacity() const { return _arr.capacity(); }

	T* at(int i) { return (i >= 0 && i < _arr.size()) ? &_arr[i] : 0; }
	T* operator[](int i) { return _arr[i]; }

	T* front() { return _arr.front(); }
	T* back() { return _arr.back(); }

	T** begin() { return _arr.begin(); }
	T** end() { return _arr.end(); }

	array<T*> _arr;

	NOCOPY(list<T>);
};