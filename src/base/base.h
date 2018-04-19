#pragma once

#include "ut.h"
#include "vec_types.h"
#include "vec_ops.h"
#include "gpu.h"
#include "audio.h"
#include "std.h"
#include "asset.h"

// input

#define KEY_MAX 256

enum : i32 {
	KEY_BACKSPACE		= 0x08,
	KEY_TAB				= 0x09,
	KEY_RETURN			= 0x0D,
	KEY_BREAK			= 0x13,
	KEY_CAPS			= 0x14,
	KEY_ESCAPE			= 0x1B,
	KEY_SPACE			= 0x20,
	KEY_PAGE_UP			= 0x21,
	KEY_PAGE_DOWN		= 0x22,
	KEY_END				= 0x23,
	KEY_HOME			= 0x24,
	KEY_LEFT			= 0x25,
	KEY_UP				= 0x26,
	KEY_RIGHT			= 0x27,
	KEY_DOWN			= 0x28,
	KEY_PRINTSCREEN		= 0x2A,
	KEY_INSERT			= 0x2D,
	KEY_DELETE			= 0x2E,
	KEY_0				= 0x30,
	KEY_1				= 0x31,
	KEY_2				= 0x32,
	KEY_3				= 0x33,
	KEY_4				= 0x34,
	KEY_5				= 0x35,
	KEY_6				= 0x36,
	KEY_7				= 0x37,
	KEY_8				= 0x38,
	KEY_9				= 0x39,
	KEY_A				= 0x41,
	KEY_B				= 0x42,
	KEY_C				= 0x43,
	KEY_D				= 0x44,
	KEY_E				= 0x45,
	KEY_F				= 0x46,
	KEY_G				= 0x47,
	KEY_H				= 0x48,
	KEY_I				= 0x49,
	KEY_J				= 0x4A,
	KEY_K				= 0x4B,
	KEY_L				= 0x4C,
	KEY_M				= 0x4D,
	KEY_N				= 0x4E,
	KEY_O				= 0x4F,
	KEY_P				= 0x50,
	KEY_Q				= 0x51,
	KEY_R				= 0x52,
	KEY_S				= 0x53,
	KEY_T				= 0x54,
	KEY_U				= 0x55,
	KEY_V				= 0x56,
	KEY_W				= 0x57,
	KEY_X				= 0x58,
	KEY_Y				= 0x59,
	KEY_Z				= 0x5A,
	KEY_LEFT_CMD		= 0x5B,
	KEY_RIGHT_CMD		= 0x5C,
	KEY_APP_MENU		= 0x5D,
	KEY_SLEEP			= 0x5F,
	KEY_NUMPAD_0		= 0x60,
	KEY_NUMPAD_1		= 0x61,
	KEY_NUMPAD_2		= 0x62,
	KEY_NUMPAD_3		= 0x63,
	KEY_NUMPAD_4		= 0x64,
	KEY_NUMPAD_5		= 0x65,
	KEY_NUMPAD_6		= 0x66,
	KEY_NUMPAD_7		= 0x67,
	KEY_NUMPAD_8		= 0x68,
	KEY_NUMPAD_9		= 0x69,
	KEY_NUMPAD_MULTIPLY	= 0x6A,
	KEY_NUMPAD_PLUS		= 0x6B,
	KEY_NUMPAD_RETURN	= 0x6C,
	KEY_NUMPAD_MINUS	= 0x6D,
	KEY_NUMPAD_PERIOD	= 0x6E,
	KEY_NUMPAD_DIVIDE	= 0x6F,
	KEY_F1				= 0x70,
	KEY_F2				= 0x71,
	KEY_F3				= 0x72,
	KEY_F4				= 0x73,
	KEY_F5				= 0x74,
	KEY_F6				= 0x75,
	KEY_F7				= 0x76,
	KEY_F8				= 0x77,
	KEY_F9				= 0x78,
	KEY_F10				= 0x79,
	KEY_F11				= 0x7A,
	KEY_F12				= 0x7B,
	KEY_NUMLOCK			= 0x90,
	KEY_SCROLL			= 0x91,
	KEY_LEFT_SHIFT		= 0xA0,
	KEY_RIGHT_SHIFT		= 0xA1,
	KEY_LEFT_CONTROL	= 0xA2,
	KEY_RIGHT_CONTROL	= 0xA3,
	KEY_LEFT_ALT		= 0xA4,
	KEY_RIGHT_ALT		= 0xA5,
	KEY_SEMICOLON		= 0xBA,
	KEY_PLUS			= 0xBB,
	KEY_COMMA			= 0xBC,
	KEY_MINUS			= 0xBD,
	KEY_PERIOD			= 0xBE,
	KEY_DIVIDE			= 0xBF,
	KEY_QUOTE			= 0xC0,
	KEY_OPEN_BRACKET	= 0xDB,
	KEY_PIPE			= 0xDC,
	KEY_CLOSE_BRACKET	= 0xDD,
	KEY_HASH			= 0xDE,
	KEY_BACK_QUOTE		= 0xDF,
};

enum : i32 {
	MOUSE_LEFT   = 1 << 0,
	MOUSE_RIGHT  = 1 << 1,
	MOUSE_MIDDLE = 1 << 2,
	MOUSE_X0     = 1 << 3,
	MOUSE_X1     = 1 << 4,
};

enum : i32 {
	PAD_A             = 1 << 0,
	PAD_B             = 1 << 1,
	PAD_X             = 1 << 2,
	PAD_Y             = 1 << 3,
	PAD_START         = 1 << 4,
	PAD_BACK          = 1 << 5,
	PAD_LEFT_BUMPER   = 1 << 6,
	PAD_RIGHT_BUMPER  = 1 << 7,
	PAD_LEFT_TRIGGER  = 1 << 8,
	PAD_RIGHT_TRIGGER = 1 << 9,
	PAD_LEFT_THUMB    = 1 << 10,
	PAD_RIGHT_THUMB   = 1 << 11,
	PAD_DPAD_UP       = 1 << 12,
	PAD_DPAD_DOWN     = 1 << 13,
	PAD_DPAD_LEFT     = 1 << 14,
	PAD_DPAD_RIGHT    = 1 << 15,
};

struct input {
	bool	key[KEY_MAX];
	bool	key_old[KEY_MAX];

	vec2i	mouse_pos;
	vec2i	mouse_rel;
	int		mouse_buttons;
	int		mouse_buttons_old;
	int		mouse_buttons_pressed;
	int		mouse_time;
	bool	mouse_active;

	bool	pad_is_present;
	vec2	pad_left;
	vec2	pad_right;
	int		pad_buttons;
	int		pad_buttons_old;
	int		pad_buttons_pressed;

	bool	start;

	int		bind_up;
	int		bind_left;
	int		bind_down;
	int		bind_right;
};

extern bool g_request_mouse_capture;
extern bool g_hide_mouse_cursor;
extern input g_input;

bool is_key_down(int key);
bool is_key_pressed(int key);

void input_init();
void input_update(bool check_for_new_pads);
void input_lost_focus();
void input_mouse_move_event(vec2i pos);
void input_mouse_relative_move_event(vec2i pos);
void input_mouse_button_event(int button, bool down);
void input_key_event(int key, bool down);