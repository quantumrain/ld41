#pragma once

struct wav_header;

struct sound {
	i16* data;
	u32 size_in_samples;
	u16 format;
	u16 sample_rate;
};

struct voice_id { u16 v; voice_id(u16 v_ = 0) : v(v_) { } operator u16() { return v; } };

enum { SOUND_LOOP = 0x1 };

void audio_init(int max_voices, int max_msgs); // max_msgs must be pow2
void audio_shutdown();

sound audio_create_sound(wav_header* h);

void audio_play(voice_id vid, sound snd, float semitones, float decibels, int flags);
void audio_stop(voice_id sid);

bool audio_is_playing(voice_id vid);

void audio_set_volume(voice_id vid, float decibels);
void audio_set_frequency(voice_id vid, float semitones);