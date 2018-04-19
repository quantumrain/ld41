#include "pch.h"
#include "tool.h"

const wchar_t* g_win_name = L"tool";
HWND g_win_hwnd;

bool name_copy(array<u8>& d, const char* s) {
	const char* p = s;
	const char* e = sz_find_last_or_end(s, '.');

	if (!d.set_size(e - s + 1))
		return false;

	memcpy(d.begin(), s, e - s);
	d[e - s] = '\0';

	_strlwr_s((char*)d.begin(), d.size());

	return true;
}

asset_bank* get_bank(asset_file_builder* afb, u32 code) {
	for(auto& b : afb->banks) {
		if (b->code == code)
			return b;
	}

	asset_bank* b = afb->banks.push_back(new asset_bank);

	if (!b)
		panic("get_bank: out of memory");

	b->code = code;

	return b;
}

asset* new_asset(asset_file_builder* afb, u32 code, const char* name) {
	asset_bank* b = get_bank(afb, code);
	asset* a = b->assets.push_back(new asset);

	if (!a)
		panic("new_asset: out of memory");

	name_copy(a->name, name);

	return a;
}

void find_assets(asset_file_builder* afb, const char* root) {
	char path[MAX_PATH];

	_snprintf_s(path, MAX_PATH, _TRUNCATE, "%s/*", root);

	WIN32_FIND_DATAA wfd;
	HANDLE f = FindFirstFileA(path, &wfd);

	if (f != INVALID_HANDLE_VALUE) {
		do {
			if ((wfd.cFileName[0] == '.') || (wfd.cFileName[0] == '_'))
				continue;

			_snprintf_s(path, MAX_PATH, _TRUNCATE, "%s\\%s", root, wfd.cFileName);

			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				find_assets(afb, path);
			}
			else {
				if      (strstr(wfd.cFileName, ".png")) import_png(afb, path, wfd.cFileName);
				else if (strstr(wfd.cFileName, ".wav")) import_wav(afb, path, wfd.cFileName);
				else if (strstr(wfd.cFileName, ".font")) import_font(afb, path, wfd.cFileName);
				else if (strstr(wfd.cFileName, ".hlsl")) import_hlsl(afb, path, wfd.cFileName);
			}
		} while(FindNextFileA(f, &wfd));

		FindClose(f);
	}
}

void write_assets(asset_file_builder* afb, const char* path) {
	// file header
	u32 total_size = 0;
	total_size += sizeof(asset_file_header);

	// bank headers
	u32 bank_header_start = total_size;
	total_size += sizeof(asset_bank_header) * afb->banks.size();

	// bank entries
	u32 bank_entry_start = total_size;
	for(auto& b : afb->banks) total_size += sizeof(asset_bank_entry) * b->assets.size();

	// name string table
	u32 name_start = total_size;
	for(auto& b : afb->banks) {
		for(auto& a : b->assets)
			total_size += a->name.size();
	}
	total_size = (total_size + 3) & ~3;

	// content
	u32 content_start = total_size;
	for(auto& b : afb->banks) {
		for(auto& a : b->assets) {
			total_size += a->data.size();
			total_size = (total_size + 3) & ~3;
		}
	}

	// alloc
	array<u8> data;
	if (!data.set_size(total_size)) {
		debug("write_assets: failed to allocate for %i bytes", total_size);
		return;
	}

	// build
	asset_file_header*	file_header	= (asset_file_header*)data.begin();
	asset_bank_header*	bank_header	= (asset_bank_header*)(data.begin() + bank_header_start);
	asset_bank_entry*	bank_entry	= (asset_bank_entry*)(data.begin() + bank_entry_start);
	char*				names		= (char*)data.begin();
	u8*					contents	= data.begin();

	file_header->bank_offset	= bank_header_start;
	file_header->bank_count		= afb->banks.size();

	u32 bank_i	= 0;
	u32 entry_i	= 0;

	u32 name_offset		= name_start;
	u32 content_offset	= content_start;

	for(auto& b : afb->banks) {
		asset_bank_header* bh = &bank_header[bank_i++];

		bh->code			= b->code;
		bh->entry_offset	= bank_entry_start + entry_i * sizeof(asset_bank_entry);
		bh->entry_count		= b->assets.size();

		for(auto& a : b->assets) {
			asset_bank_entry* be = &bank_entry[entry_i++];

			be->name_offset = name_offset;
			be->data_offset = content_offset;

			memcpy(names + name_offset, a->name.begin(), a->name.size());
			memcpy(contents + content_offset, a->data.begin(), a->data.size());

			name_offset		+= a->name.size();
			content_offset	+= a->data.size();

			content_offset = (content_offset + 3) & ~3;
		}
	}

	// write

	HANDLE f = CreateFileA(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	if (f == INVALID_HANDLE_VALUE) {
		debug("write_assets: failed to create file - %s", path);
		return;
	}

	DWORD written;

	if (!WriteFile(f, data.begin(), total_size, &written, 0)) {
		debug("write_assets: failed while writing %i bytes", total_size);
	}

	CloseHandle(f);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	char root[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, root);

	asset_file_builder afb;

	find_assets(&afb, "data");
	find_assets(&afb, "shaders");
	write_assets(&afb, "data\\assets.dat");

	return 0;
}