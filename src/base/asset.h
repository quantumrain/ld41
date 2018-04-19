#pragma once

#define ASSET_CODE(a, b, c, d) (((u32)(a)) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

#define ASSET_PNG	ASSET_CODE('P', 'N', 'G', '_')
#define ASSET_WAV	ASSET_CODE('W', 'A', 'V', '_')
#define ASSET_FONT	ASSET_CODE('F', 'O', 'N', 'T')
#define ASSET_HLSL	ASSET_CODE('H', 'L', 'S', 'L')

enum {
	WAV_FORMAT_MONO_8,
	WAV_FORMAT_MONO_16,
	WAV_FORMAT_STEREO_8,
	WAV_FORMAT_STEREO_16,
	WAV_FORMAT_COUNT
};

struct asset_file_header {
	u32 bank_offset;
	u32 bank_count;
	// perm_size, temp_size, stream_size - split data based on lifetime?
};

struct asset_bank_header {
	u32 code;
	u32 entry_offset;
	u32 entry_count;
};

struct asset_bank_entry {
	u32 name_offset;
	u32 data_offset;
};

struct raw_header {
	u32 size;
};

struct wav_header {
	u32 size;
	u16 samples;
	u16 format;
};

struct font_header {
	u8 glyph_width[128];
};

struct hlsl_header {
	u32 vs_size;
	u32 ps_size;
};