#include "pch.h"
#include "platform.h"
#include "base.h"

#define SAMPLE_RATE			48000
#define FRAME_RATE			60

#define CURSOR_SHIFT		(10u)
#define CURSOR_SCALE		(1u << CURSOR_SHIFT)
#define CURSOR_MASK			(CURSOR_SCALE - 1u)
#define CURSOR_SCALE_INV	(1.0f / (f32)CURSOR_SCALE)

#define MIN_FREQ_RATIO		(1u)
#define MAX_FREQ_RATIO		(1024u << CURSOR_SHIFT)

struct mono_voice {
	i16* data;
	u32 format;
	u32 sample_rate;

	u32 cursor;
	u32 cursor_max;
	u32 cursor_step;

	f32 volume;
	f32 target_volume;

	u32 flags;
	u32 play_id;

	mono_voice() : data(), format(), sample_rate(), cursor(), cursor_max(), cursor_step(), volume(), target_volume(), flags(), play_id() { }
};

struct audio_msg {
	enum {
		PLAY,
		STOP,
		SET_VOLUME,
		SET_FREQ
	};

	u32 msg;
	u32 voice;
	float value;
	mono_voice data;
};

struct audio {
	IMMDeviceEnumerator* mm_device_enumerator;
	IMMDevice*           mm_device;
	IAudioClient*        audio_client;
	IAudioRenderClient*  audio_render_client;
	HANDLE               wake_audio_event;
	HANDLE               audio_thread;

	u32 size_of_audio_buffer_in_samples;
	u32 latency_in_samples;

	array<mono_voice>	voice;
	array<audio_msg>	msg;
	volatile u32		msg_read;
	volatile u32		msg_write;
	volatile u32*		voice_play_id;

	volatile u32		stop_audio_thread;
};

audio g_audio;

// sound sys

void audio_init(int max_voices, int max_msgs) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	HRESULT hr;

	if (FAILED(hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, IID_PPV_ARGS(&g_audio.mm_device_enumerator)))) {
		debug("audio_init: CoCreateInstance(MMDeviceEnumerator) failed %08X", hr);
		return;
	}

	if (FAILED(hr = g_audio.mm_device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &g_audio.mm_device))) {
		debug("audio_init: GetDefaultAudioEndpoint failed %08X", hr);
		return;
	}

	if (FAILED(hr = g_audio.mm_device->Activate(__uuidof(g_audio.audio_client), CLSCTX_ALL, 0, (void**)&g_audio.audio_client))) {
		debug("audio_init: mm_device->Activate() failed %08X", hr);
		return;
	}

	if ((g_audio.wake_audio_event = CreateEvent(0, FALSE, FALSE, 0)) == 0) {
		debug("audio_init: CreateEvent() failed");
		return;
	}

	/*{
		WAVEFORMATEXTENSIBLE* default_wfx = 0;

		if (FAILED(hr = g_audio.audio_client->GetMixFormat((WAVEFORMATEX**)&default_wfx))) {
			debug("audio_init: mm_device->GetMixFormat() failed %08X", hr);
			return;
		}

		if (default_wfx) {
			debug("audio: init: default_wfx - chs(%i) rate(%i) bits(%i)", default_wfx->Format.nChannels, default_wfx->Format.nSamplesPerSec, default_wfx->Format.wBitsPerSample);

			if (default_wfx->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
				const char* sf = "unknown";

				if      (!memcmp(&default_wfx->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(default_wfx->SubFormat))) sf = "float";
				else if (!memcmp(&default_wfx->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM,        sizeof(default_wfx->SubFormat))) sf = "pcm";

				debug("audio: init: default_wfx - subformat(%s) chmask(%08X) bits(%i)", sf, default_wfx->dwChannelMask, default_wfx->Samples.wValidBitsPerSample);
			}
		}

		// TODO: respect default_wfx

		CoTaskMemFree(default_wfx);
	}*/

	WAVEFORMATEXTENSIBLE wfx = { 0 };

	wfx.Format.wFormatTag			= WAVE_FORMAT_EXTENSIBLE;
	wfx.Format.nChannels			= 2;
	wfx.Format.nSamplesPerSec		= SAMPLE_RATE;
	wfx.Format.nAvgBytesPerSec		= sizeof(f32) * wfx.Format.nChannels * wfx.Format.nSamplesPerSec;
	wfx.Format.nBlockAlign			= sizeof(f32) * wfx.Format.nChannels;
	wfx.Format.wBitsPerSample		= sizeof(f32) * 8;
	wfx.Format.cbSize				= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample	= sizeof(f32) * 8;
	wfx.dwChannelMask				= KSAUDIO_SPEAKER_STEREO;
	wfx.SubFormat					= KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

	i64 ns100_per_second	= 10000000LL;
	i64 video_framerate		= FRAME_RATE;
	i64 desired_buffer		= (ns100_per_second * 3) / (video_framerate * 2); // 1.5 frames of buffer/latency

	#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM     0x80000000
	#define AUDCLT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000

	if (FAILED(hr = g_audio.audio_client->Initialize(
			AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
			desired_buffer, 0, (WAVEFORMATEX*)&wfx, 0
	))) {
		debug("audio_init: audio_client->Initialize() failed %08X", hr);
		return;
	}

	if (FAILED(hr = g_audio.audio_client->SetEventHandle(g_audio.wake_audio_event))) {
		debug("audio_init: audio_client->SetEventHandle() failed %08X", hr);
		return;
	}

	if (FAILED(hr = g_audio.audio_client->GetBufferSize(&g_audio.size_of_audio_buffer_in_samples))) {
		debug("audio_init: audio_client->GetBufferSize() failed %08X", hr);
		return;
	}

	i64 latency_in_ns100 = 0;
	if (FAILED(hr = g_audio.audio_client->GetStreamLatency(&latency_in_ns100))) {
		debug("audio_init: audio_client->GetBufferSize() failed %08X", hr);
		return;
	}

	i64 ns100_per_sample = ns100_per_second / SAMPLE_RATE;
	g_audio.latency_in_samples = (u32)((latency_in_ns100 + ns100_per_sample - 1) / ns100_per_sample);

	if (FAILED(hr = g_audio.audio_client->GetService(__uuidof(g_audio.audio_render_client), (void**)&g_audio.audio_render_client))) {
		debug("audio_init: audio_client->GetService() failed %08X", hr);
		return;
	}

	if (FAILED(hr = g_audio.audio_client->Start())) {
		debug("audio_init: audio_client->Start() failed %08X", hr);
		return;
	}

	g_audio.voice.set_size(max_voices);
	g_audio.msg.set_size(max_msgs);

	g_audio.voice_play_id = new u32[max_voices];

	DWORD WINAPI audio_thread(void*);

	if ((g_audio.audio_thread = CreateThread(0, 0, audio_thread, 0, 0, 0)) == NULL) {
		debug("audio_init: CreateThread() failed");
		return;
	}
}

void audio_shutdown()  {
	if (g_audio.audio_thread) {
		g_audio.stop_audio_thread = 1;
		SetEvent(g_audio.wake_audio_event);
		WaitForSingleObject(g_audio.audio_thread, INFINITE);
		CloseHandle(g_audio.audio_thread);
	}

	if (g_audio.audio_client) {
		g_audio.audio_client->Stop();
	}

	if (g_audio.audio_render_client) {
		g_audio.audio_render_client->Release();
	}

	if (g_audio.audio_client) {
		g_audio.audio_client->Release();
	}

	if (g_audio.wake_audio_event) {
		CloseHandle(g_audio.wake_audio_event);
	}

	if (g_audio.mm_device) {
		g_audio.mm_device->Release();
	}

	if (g_audio.mm_device_enumerator) {
		g_audio.mm_device_enumerator->Release();
	}

	g_audio.voice.free();
	g_audio.msg.free();

	delete [] g_audio.voice_play_id;
}

sound audio_create_sound(wav_header* h) {
	sound s;

	s.data				= (i16*)&h[1];
	s.size_in_samples	= 0;
	s.sample_rate		= h->samples;
	s.format			= h->format;

	switch(h->format) {
		case WAV_FORMAT_MONO_8:
			s.size_in_samples = h->size;
		break;

		case WAV_FORMAT_MONO_16:
			s.size_in_samples = h->size / 2;
		break;

		default:
			assert(0 && "Unsupported audio format");
		break;
	}

	return s;
}

void audio_play(voice_id vid, sound snd, float semitones, float decibels, int flags) {
	if ((vid <= 0) || (vid > g_audio.voice.size()))
		return;

	if ((g_audio.msg_write - g_audio.msg_read) >= (u32)g_audio.msg.size())
		return;

	audio_msg* m = &g_audio.msg[g_audio.msg_write & (g_audio.msg.size() - 1)];

	float rate		= powf(2.0f, semitones / 12.0f);
	float volume	= powf(10.0f, decibels / 20.0f);

	m->msg					= audio_msg::PLAY;
	m->voice				= vid - 1;
	m->data.data			= snd.data;
	m->data.format			= snd.format;
	m->data.sample_rate		= snd.sample_rate;
	m->data.cursor			= 0;
	m->data.cursor_max		= snd.size_in_samples << CURSOR_SHIFT;
	m->data.cursor_step		= clamp((u32)((snd.sample_rate * rate * (CURSOR_SCALE / (f32)SAMPLE_RATE)) + 0.5f), MIN_FREQ_RATIO, min(m->data.cursor_max, MAX_FREQ_RATIO));
	m->data.volume			= volume;
	m->data.target_volume	= volume;
	m->data.flags			= flags;
	m->data.play_id			= g_audio.msg_write | 0x80000000;

	g_audio.voice_play_id[vid - 1] = g_audio.msg_write | 0x80000000;

	_WriteBarrier();
	_InterlockedIncrement((volatile long*)&g_audio.msg_write);
}

void audio_stop(voice_id vid) {
	if ((vid <= 0) || (vid > g_audio.voice.size()))
		return;

	if ((g_audio.msg_write - g_audio.msg_read) >= (u32)g_audio.msg.size())
		return;

	audio_msg* m = &g_audio.msg[g_audio.msg_write & (g_audio.msg.size() - 1)];

	m->msg		= audio_msg::STOP;
	m->voice	= vid - 1;

	g_audio.voice_play_id[vid - 1] = 0;

	_WriteBarrier();
	_InterlockedIncrement((volatile long*)&g_audio.msg_write);
}

bool audio_is_playing(voice_id vid) {
	if ((vid <= 0) || (vid > g_audio.voice.size()))
		return false;

	return g_audio.voice_play_id[vid - 1] != 0;
}

void audio_set_volume(voice_id vid, float decibels) {
	if ((vid <= 0) || (vid > g_audio.voice.size()))
		return;

	if ((g_audio.msg_write - g_audio.msg_read) >= (u32)g_audio.msg.size())
		return;

	audio_msg* m = &g_audio.msg[g_audio.msg_write & (g_audio.msg.size() - 1)];

	m->msg		= audio_msg::SET_VOLUME;
	m->voice	= vid - 1;
	m->value	= powf(10.0f, decibels / 20.0f);

	_WriteBarrier();
	_InterlockedIncrement((volatile long*)&g_audio.msg_write);
}

void audio_set_frequency(voice_id vid, float semitones) {
	if ((vid <= 0) || (vid > g_audio.voice.size()))
		return;

	if ((g_audio.msg_write - g_audio.msg_read) >= (u32)g_audio.msg.size())
		return;

	audio_msg* m = &g_audio.msg[g_audio.msg_write & (g_audio.msg.size() - 1)];

	m->msg		= audio_msg::SET_FREQ;
	m->voice	= vid - 1;
	m->value	= powf(2.0f, semitones / 12.0f);

	_WriteBarrier();
	_InterlockedIncrement((volatile long*)&g_audio.msg_write);
}

//
// todo: clean up
//

bool mix_voice_mono_i8(f32* output_buffer, u32 samples_to_output, mono_voice* voice) {
	u8*		sample_data		= (u8*)voice->data;
	u32		cursor			= voice->cursor;
	u32		cursor_max		= voice->cursor_max;
	u32		cursor_step		= voice->cursor_step;
	f32		volume			= voice->volume;
	f32		delta_volume	= (voice->target_volume - voice->volume) / samples_to_output;

	voice->volume = voice->target_volume;

	u32 fractional_samples_to_output = samples_to_output * cursor_step;

	for(;;) {
		u32 fractional_samples_to_mix		= min(fractional_samples_to_output, cursor_max - cursor);
		u32 last_fractional_sample_to_mix	= cursor + fractional_samples_to_mix;

		while(cursor < last_fractional_sample_to_mix) {
			f32 v0	= ((sample_data[(cursor >> CURSOR_SHIFT) + 0] - 128) * 256.0f) * volume;
			f32 v1	= ((sample_data[(cursor >> CURSOR_SHIFT) + 1] - 128) * 256.0f) * volume;
			f32 v	= v0 + (v1 - v0) * ((cursor & CURSOR_MASK) * CURSOR_SCALE_INV);

			output_buffer[0] += v;
			output_buffer[1] += v;

			cursor							+= cursor_step;
			output_buffer					+= 2;
			volume							+= delta_volume;
		}

		if (cursor >= cursor_max) {
			if (!(voice->flags & SOUND_LOOP)) {
				voice->data = 0;
				return false;
			}

			// todo: allow looping into a different sound? would be useful for streaming/music
			cursor -= cursor_max;
			fractional_samples_to_output -= cursor; // align to whole sample boundry
		}

		fractional_samples_to_output -= fractional_samples_to_mix;

		if (fractional_samples_to_output == 0) {
			voice->cursor = cursor;
			return true;
		}
	}
}

bool mix_voice_mono_i16(f32* output_buffer, u32 samples_to_output, mono_voice* voice) {
	i16*	sample_data		= voice->data;
	u32		cursor			= voice->cursor;
	u32		cursor_max		= voice->cursor_max;
	u32		cursor_step		= voice->cursor_step;
	f32		volume			= voice->volume;
	f32		delta_volume	= (voice->target_volume - voice->volume) / samples_to_output;

	voice->volume = voice->target_volume;

	u32 fractional_samples_to_output = samples_to_output * cursor_step;

	for(;;) {
		u32 fractional_samples_to_mix		= min(fractional_samples_to_output, cursor_max - cursor);
		u32 last_fractional_sample_to_mix	= cursor + fractional_samples_to_mix;

		while(cursor < last_fractional_sample_to_mix) {
			f32 v0	= sample_data[(cursor >> CURSOR_SHIFT) + 0] * volume;
			f32 v1	= sample_data[(cursor >> CURSOR_SHIFT) + 1] * volume;
			f32 v	= v0 + (v1 - v0) * ((cursor & CURSOR_MASK) * CURSOR_SCALE_INV);

			output_buffer[0] += v;
			output_buffer[1] += v;

			cursor							+= cursor_step;
			output_buffer					+= 2;
			volume							+= delta_volume;
		}

		if (cursor >= cursor_max) {
			if (!(voice->flags & SOUND_LOOP)) {
				voice->data = 0;
				return false;
			}

			// todo: allow looping into a different sound? would be useful for streaming/music
			cursor -= cursor_max;
			fractional_samples_to_output -= cursor; // align to whole sample boundry
		}

		fractional_samples_to_output -= fractional_samples_to_mix;

		if (fractional_samples_to_output == 0) {
			voice->cursor = cursor;
			return true;
		}
	}
}

void mix_frame(f32* output_buffer, i32 samples_to_output) {
	for(int i = 0; i < g_audio.voice.size(); i++) {
		mono_voice* v = &g_audio.voice[i];

		if (!v->data)
			continue;

		if (v->format == WAV_FORMAT_MONO_8) {
			if (!mix_voice_mono_i8(output_buffer, samples_to_output, v)) {
				_InterlockedCompareExchange((volatile long*)&g_audio.voice_play_id[i], 0, v->play_id);
			}
		}
		else if (v->format == WAV_FORMAT_MONO_16) {
			if (!mix_voice_mono_i16(output_buffer, samples_to_output, v)) {
				_InterlockedCompareExchange((volatile long*)&g_audio.voice_play_id[i], 0, v->play_id);
			}
		}
	}
}

void mix_audio(f32* output_buffer, u32 samples_to_output) {
	const int SAMPLES_PER_FRAME = SAMPLE_RATE / FRAME_RATE;
	static f32 sample_data[2 * SAMPLES_PER_FRAME];
	static u32 samples_remaining;

	for(;;) {
		if (samples_remaining == 0) {
			memset(sample_data, 0, sizeof(sample_data));
			mix_frame(sample_data, SAMPLES_PER_FRAME);
			samples_remaining = SAMPLES_PER_FRAME;
		}

		u32 samples_to_copy = min(samples_remaining, samples_to_output);

		f32* s = &sample_data[2 * (SAMPLES_PER_FRAME - samples_remaining)];

		for(u32 i = 0; i < samples_to_copy; i++) {
			*output_buffer++ = clamp(*s++ / 32768.0f, -1.0f, 1.0f);
			*output_buffer++ = clamp(*s++ / 32768.0f, -1.0f, 1.0f);
		}

		samples_remaining -= samples_to_copy;
		samples_to_output -= samples_to_copy;

		if (samples_to_output == 0)
			break;
	}
}

DWORD WINAPI audio_thread(void*) {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while(!g_audio.stop_audio_thread) {
		WaitForSingleObject(g_audio.wake_audio_event, 1000 / FRAME_RATE);

		for(;;) {
			u32 msg_read = g_audio.msg_read;

			if (msg_read >= g_audio.msg_write)
				break;

			_ReadBarrier();

			audio_msg m = g_audio.msg[msg_read & (g_audio.msg.size() - 1)];
			mono_voice* v = &g_audio.voice[m.voice];

			switch(m.msg) {
				case audio_msg::PLAY:		*v = m.data;																																				break;
				case audio_msg::STOP:		v->data = 0;																																				break;
				case audio_msg::SET_VOLUME:	v->target_volume = m.value;																																	break;
				case audio_msg::SET_FREQ:	v->cursor_step = clamp((u32)((v->sample_rate * m.value * (CURSOR_SCALE / (f32)SAMPLE_RATE)) + 0.5f), MIN_FREQ_RATIO, min(v->cursor_max, MAX_FREQ_RATIO));	break;
			}

			_InterlockedIncrement((volatile long*)&g_audio.msg_read);
		}

		HRESULT hr;

		u32 samples_still_to_play = 0;
		if (FAILED(g_audio.audio_client->GetCurrentPadding(&samples_still_to_play))) {
			debug("audio_client->GetCurrentPadding");
		}

		if (samples_still_to_play == 0) {
			static bool skipped_first;

			if (skipped_first)
				debug("audio probably skipped?");
			else
				skipped_first = false;
		}

		u32 samples_to_mix = g_audio.size_of_audio_buffer_in_samples - samples_still_to_play;

		if (samples_to_mix > 0) {
			u8* buffer = 0;
			if (FAILED(hr = g_audio.audio_render_client->GetBuffer(samples_to_mix, &buffer))) {
				debug("audio_init: audio_render_client->GetBuffer() failed %08X", hr);
			}
			else {
				mix_audio((f32*)buffer, samples_to_mix);

				if (FAILED(hr = g_audio.audio_render_client->ReleaseBuffer(samples_to_mix, 0))) {
					debug("audio_init: audio_render_client->ReleaseBuffer() failed %08X", hr);
				}
			}
		}
	}

	return 0;
}