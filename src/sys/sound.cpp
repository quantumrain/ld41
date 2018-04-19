#include "pch.h"
#include "base.h"
#include "sys.h"

// todo
// - panning
// - loop count / forever loop / cancel
// - volume / panning of inflight sounds
// - config file for cooldown, cut off, multiple sounds, etc
// - should store all sounds played during a frame and coalesce similar ones

#define SOUND_FX_NAME_LENGTH 24

struct voice_fx {
	voice_id id;
	u64 last_played;

	voice_fx(voice_id id_) : id(id_), last_played() { }
};

struct sound_fx {
	char name[SOUND_FX_NAME_LENGTH];
	array<sound> variant;
	array<voice_fx> voice;
	int cooldown;
	int max_cooldown;
	int max_voices;

	sound_fx() : cooldown(), max_cooldown(), max_voices() { }
};

u16 g_sfx_map[sfx::_MAX];
array<sound_fx> g_sound_fx;
int g_sound_num_voices_allocated;
random g_sound_rand(1);

int find_sound_fx(const char* name) {
	for(int i = 0; i < g_sound_fx.size(); i++) {
		if (!strcmp(g_sound_fx[i].name, name))
			return i;
	}
	return -1;
}

sound_fx* acquire_sound_fx(const char* name) {
	for(auto& fx : g_sound_fx) {
		if (!strcmp(fx.name, name))
			return &fx;
	}

	sound_fx* fx = g_sound_fx.push_back();

	if (fx)
		sz_copy(fx->name, name, sizeof(fx->name));

	return fx;
}

void sound_init() {
	find_asset_data fad;
	const char* name;

	while(wav_header* wav = (wav_header*)find_next_asset(&fad, ASSET_WAV, &name)) {
		char prefix[SOUND_FX_NAME_LENGTH];

		const char* p = sz_find_last_or_end(name, '_');

		if (*p && ((p[1] >= '0') && (p[1] <= '9')))
			sz_copy(prefix, name, p, SOUND_FX_NAME_LENGTH);
		else
			sz_copy(prefix, name, SOUND_FX_NAME_LENGTH);

		if (sound_fx* fx = acquire_sound_fx(prefix)) {
			fx->variant.push_back(audio_create_sound(wav));
		}
	}
}

void define_sound(sfx s, const char* name, int max_voices, int max_cooldown) {
	int fxi = find_sound_fx(name);

	if (fxi < 0) {
		debug("define_sound: no variants for sound fx - %s", name);
		return;
	}

	g_sfx_map[(int)s] = 1 + fxi;

	sound_fx* fx = &g_sound_fx[fxi];

	if (fx->max_voices) {
		debug("define_sound: sound already mapped %s", name);
		return;
	}

	fx->max_voices		= max_voices;
	fx->max_cooldown	= max_cooldown;

	for(int i = 0; i < fx->max_voices; i++)
		fx->voice.push_back(voice_id(++g_sound_num_voices_allocated));
}

void sound_step() {
	for(auto& fx : g_sound_fx) {
		if (fx.cooldown > 0)
			fx.cooldown--;
	}
}

voice_id sound_play(sfx s, float semitones, float decibels, int flags) {
	int si = g_sfx_map[(int)s] - 1;

	if ((si < 0) || (si > g_sound_fx.size()))
		return 0;

	sound_fx* fx = &g_sound_fx[si];

	if (fx->cooldown > 0)
		return 0;

	if (fx->variant.empty())
		return 0;

	voice_fx* best = 0;
	u64 best_played = 0;

	for(int i = 0; i < fx->voice.size(); i++) {
		voice_fx* vf = &fx->voice[i];

		if (!audio_is_playing(vf->id)) {
			best = vf;
			break;
		}

		if (!best || (vf->last_played < best_played)) { // todo: will break on timer wrap
			best = vf;
			best_played = vf->last_played;
		}
	}

	if (best) {
		sound snd = fx->variant[g_sound_rand.rand(fx->variant.size())];

		audio_play(best->id, snd, semitones, decibels, flags);

		fx->cooldown		= fx->max_cooldown;
		best->last_played	= timer_ticks();
	}

	return best->id;
}

void sound_stop(sfx s) {
	int si = g_sfx_map[(int)s] - 1;

	if ((si < 0) || (si > g_sound_fx.size()))
		return;

	sound_fx* fx = &g_sound_fx[si];

	for(auto& v : fx->voice)
		audio_stop(v.id);
}