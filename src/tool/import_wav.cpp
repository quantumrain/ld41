#include "pch.h"
#include "tool.h"

/*
	todo: sound_bank?
*/

#define FOURCC(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

#define RIFF_FOURCC_RIFF	FOURCC('R', 'I', 'F', 'F')
#define RIFF_FOURCC_DATA	FOURCC('d', 'a', 't', 'a')
#define RIFF_FOURCC_FMT		FOURCC('f', 'm', 't', ' ')
#define RIFF_FOURCC_WAVE	FOURCC('W', 'A', 'V', 'E')

struct riff_chunk {
	const u8* data;
	u32 size;

	riff_chunk() : data(), size() { }
	riff_chunk(const u8* data_, u32 size_) : data(data_), size(size_) { }
	operator bool() const { return data != 0; }
};

riff_chunk riff_find_chunk(const u8* riff_data, u32 riff_size, DWORD fourcc) {
	u32 offset = 4;

	while(offset < riff_size) {
		u32 code = *(const u32*)(riff_data + offset);
		i32 size = *(const u32*)(riff_data + offset + 4);

		if (code == fourcc)
			return riff_chunk(riff_data + offset + 8, size);

		offset += 8 + size;
	}

	return riff_chunk();
}

void import_wav(asset_file_builder* afb, const char* path, const char* name) {
	file_buf fb;

	if (!load_file(&fb, path)) {
		debug("import_wav: failed to load - %s", path);
		return;
	}

	u32* p = (u32*)fb.data;

	if (fb.size <= 12 || p[0] != RIFF_FOURCC_RIFF || p[2] != RIFF_FOURCC_WAVE) {
		debug("import_wav: unsupported file - %s", path);
		return;
	}

	u32 riff_size = p[1];

	if (riff_size > ((u32)fb.size - 8)) {
		debug("import_wav: corrupt file - %s", path);
		return;
	}

	riff_chunk ch_fmt = riff_find_chunk(fb.data + 8, riff_size, RIFF_FOURCC_FMT);
	riff_chunk ch_data = riff_find_chunk(fb.data + 8, riff_size, RIFF_FOURCC_DATA);

	if (!ch_fmt || !ch_data || ch_fmt.size < 16 || ch_data.size > riff_size) {
		debug("import_wav: missing required chunks - %s", path);
		return;
	}

	WAVEFORMATEXTENSIBLE* wfx = (WAVEFORMATEXTENSIBLE*)ch_fmt.data;

	if (wfx->Format.wFormatTag != WAVE_FORMAT_PCM) {
		debug("import_wav: unsupported format (%X) - %s", wfx->Format.wFormatTag, path);
		return;
	}

	if (wfx->Format.nSamplesPerSec > 0xFFFF) {
		debug("import_wav: samples per sec > 65535 (%i) - %s", wfx->Format.nSamplesPerSec, path);
		return;
	}

	if ((wfx->Format.nChannels != 1) && (wfx->Format.nChannels != 2)) {
		debug("import_wav: unsupported channel count (%i) - %s", wfx->Format.nChannels, path); 
		return;
	}

	if ((wfx->Format.wBitsPerSample != 8) && (wfx->Format.wBitsPerSample != 16)) {
		debug("import_wav: unsupported bits per sample count (%i) - %s", wfx->Format.wBitsPerSample, path); 
		return;
	}

	if ((wfx->Format.wBitsPerSample == 8) && (wfx->Format.nBlockAlign != 1)) {
		debug("import_wav: bits per sample == 8 but block align (%i) != 1 - %s", wfx->Format.nBlockAlign, path);
		return;
	}

	if ((wfx->Format.wBitsPerSample == 16) && (wfx->Format.nBlockAlign != 2)) {
		debug("import_wav: bits per sample == 16 but block align (%i) != 2 - %s", wfx->Format.nBlockAlign, path);
		return;
	}

	u32 format = 0;

	if (wfx->Format.nChannels == 1)
		format = (wfx->Format.wBitsPerSample == 8) ? WAV_FORMAT_MONO_8 : WAV_FORMAT_MONO_16;
	else
		format = (wfx->Format.wBitsPerSample == 8) ? WAV_FORMAT_STEREO_8 : WAV_FORMAT_STEREO_16;

	asset* a = new_asset(afb, ASSET_WAV, name);

	if (!a) {
		debug("import_wav: failed to allocate asset - %s", path);
		return;
	}

	// +1 sample because we duplicate the last sample of the sound to avoid having to edge check in the mixer
	// todo: This can be extended to supporting looping/streaming sounds by padding the end with whatever the next samples should be
	//       I'll need to know at resource pack time whether a sound will be looped/streamed or not
	//       This could potentially support loop points by splitting the sound internally into three parts and then chaining them together
	u32 total_size = sizeof(wav_header) + ch_data.size + wfx->Format.nBlockAlign;

	if (!a->data.set_size(total_size)) {
		debug("import_wav: failed to allocate asset data - %s", path);
		return;
	}

	wav_header*	h = (wav_header*)a->data.begin();
	u8*			d = a->data.begin() + sizeof(wav_header);

	h->size		= ch_data.size;
	h->samples	= (u16)wfx->Format.nSamplesPerSec;
	h->format	= format;

	memcpy(d, ch_data.data, ch_data.size);
	memcpy(d + ch_data.size, ch_data.data + ch_data.size - wfx->Format.nBlockAlign, wfx->Format.nBlockAlign);
}