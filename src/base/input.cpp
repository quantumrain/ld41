#include "pch.h"
#include "platform.h"
#include "base.h"

vec2 remap_stick(i32 ix, i32 iy);

struct raw_input {
	bool  key[KEY_MAX];
	vec2i mouse_pos;
	vec2i mouse_rel;
	int   mouse_buttons;
};

raw_input g_raw_input;
input     g_input;

bool is_key_down(int key)    { return g_input.key[(u8)key]; }
bool is_key_pressed(int key) { return g_input.key[(u8)key] && !g_input.key_old[(u8)key]; }

decltype(&XInputGetState) xinput_get_state;

void input_init() {
	g_input.bind_up    = MapVirtualKey(0x11, MAPVK_VSC_TO_VK);
	g_input.bind_left  = MapVirtualKey(0x1E, MAPVK_VSC_TO_VK);
	g_input.bind_down  = MapVirtualKey(0x1F, MAPVK_VSC_TO_VK);
	g_input.bind_right = MapVirtualKey(0x20, MAPVK_VSC_TO_VK);

	HMODULE xinput = LoadLibrary(L"xinput1_4.dll");
	if (!xinput) xinput = LoadLibrary(L"xinput1_3.dll");
	if (!xinput) xinput = LoadLibrary(L"xinput9_1_0.dll");
	if (xinput) xinput_get_state = (decltype(&XInputGetState))GetProcAddress(xinput, "XInputGetState");
}

void input_update(bool check_for_new_pads) {
	memcpy(g_input.key_old, g_input.key,     sizeof(g_input.key_old));
	memcpy(g_input.key,     g_raw_input.key, sizeof(g_input.key));

	vec2i old_mouse_pos = g_input.mouse_pos;
	vec2i old_mouse_rel = g_input.mouse_rel;

	g_input.mouse_pos             = g_raw_input.mouse_pos;
	g_input.mouse_buttons_old     = g_input.mouse_buttons;
	g_input.mouse_buttons         = g_raw_input.mouse_buttons;
	g_input.mouse_buttons_pressed = g_input.mouse_buttons & ~g_input.mouse_buttons_old;

	g_input.mouse_rel             = g_raw_input.mouse_rel;
	g_raw_input.mouse_rel         = vec2i(); // TODO: atomic

	if (xinput_get_state) {
		if (check_for_new_pads || g_input.pad_is_present) {
			XINPUT_STATE state = { };

			if (xinput_get_state(0, &state) == ERROR_SUCCESS) {
				g_input.pad_is_present  = true;
				g_input.pad_left        = remap_stick(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
				g_input.pad_right       = remap_stick(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
				g_input.pad_buttons_old = g_input.pad_buttons;
				g_input.pad_buttons     = (
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_A             ) != 0)  ? PAD_A             : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_B             ) != 0)  ? PAD_B             : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_X             ) != 0)  ? PAD_X             : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y             ) != 0)  ? PAD_Y             : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_START         ) != 0)  ? PAD_START         : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK          ) != 0)  ? PAD_BACK          : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) != 0)  ? PAD_LEFT_BUMPER   : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)  ? PAD_RIGHT_BUMPER  : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB    ) != 0)  ? PAD_LEFT_THUMB    : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB   ) != 0)  ? PAD_RIGHT_THUMB   : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP       ) != 0)  ? PAD_DPAD_UP       : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN     ) != 0)  ? PAD_DPAD_DOWN     : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT     ) != 0)  ? PAD_DPAD_LEFT     : 0) |
						(((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT    ) != 0)  ? PAD_DPAD_RIGHT    : 0) |
						((state.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? PAD_LEFT_TRIGGER  : 0) |
						((state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? PAD_RIGHT_TRIGGER : 0)
					);
				g_input.pad_buttons_pressed = g_input.pad_buttons & ~g_input.pad_buttons_old;
			}
			else {
				g_input.pad_is_present      = false;
				g_input.pad_left            = vec2();
				g_input.pad_right           = vec2();
				g_input.pad_buttons         = 0;
				g_input.pad_buttons_old     = 0;
				g_input.pad_buttons_pressed = 0;
			}
		}
	}

	if ((old_mouse_pos == g_input.mouse_pos) && (old_mouse_rel == g_input.mouse_rel) && !g_input.mouse_buttons) {
		if (++g_input.mouse_time > 60) {
			if ((length_sq(g_input.pad_right) > 0.0f) || is_key_down(KEY_LEFT) || is_key_down(KEY_RIGHT) || is_key_down(KEY_UP) || is_key_down(KEY_DOWN))
				g_input.mouse_active = false;
		}
		else
			g_input.mouse_active = true;
	}
	else {
		g_input.mouse_time   = 0;
		g_input.mouse_active = true;
	}

	g_input.start = is_key_pressed(KEY_SPACE) || is_key_pressed(KEY_RETURN) || (g_input.mouse_buttons_pressed & PAD_A) || (g_input.pad_buttons_pressed & MOUSE_LEFT);
}

void input_lost_focus()                              { memset(&g_raw_input, 0, sizeof(g_raw_input)); }
void input_mouse_move_event(vec2i pos)               { g_raw_input.mouse_pos     = pos; }
void input_mouse_relative_move_event(vec2i pos)      { g_raw_input.mouse_rel    += pos; }
void input_mouse_button_event(int button, bool down) { g_raw_input.mouse_buttons = (g_raw_input.mouse_buttons & ~(1 << button)) | (down << button); }
void input_key_event(int key, bool down)             { g_raw_input.key[(u8)key]  = down; }

vec2 remap_stick(i32 ix, i32 iy) {
	vec2 stick(ix / 32768.0f, iy / -32768.0f);

	float d = length(stick);

	const f32 inner_rim = 0.3f;
	const f32 outer_rim = 0.1f;

	if (d < inner_rim)
		return vec2();

	float dd = square(saturate((d - inner_rim) / (1.0f - inner_rim - outer_rim)));

	return stick * (dd / d);
}