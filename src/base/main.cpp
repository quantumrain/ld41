#include "pch.h"
#include "platform.h"
#include "base.h"

const vec2i DEFAULT_VIEW_SIZE(1280, 720);

enum class mouse_capture {
	FREE,
	CAPTURED,
	LOST
};

bool g_request_mouse_capture = true;
mouse_capture g_mouse_capture_state = mouse_capture::LOST;
bool g_hide_mouse_cursor = false;

extern const wchar_t* g_win_name;
volatile bool g_win_quit = false;
HWND g_win_hwnd;
bool g_win_focus;
int g_win_modal;
bool g_win_fullscreen;
WINDOWPLACEMENT g_win_old_placement;
HDEVNOTIFY g_dev_notify;
int g_dev_notify_id;

#define WM_APP_UPDATE_MOUSE_CURSOR (WM_APP + 1)

void frame_init();
void frame_init_view(vec2i view_size);
void frame_step(vec2 size);

DWORD WINAPI game_thread_proc(void*) {
	vec2i view_size      = { -1, -1 };
	bool  old_hide_mouse = g_hide_mouse_cursor;
	int   old_notify_id  = -1;

	while(!g_win_quit) {
		// recreate buffers if window size changes

		RECT rc;
		GetClientRect(g_win_hwnd, &rc);

		vec2i new_view_size(max<int>(rc.right - rc.left, 64), max<int>(rc.bottom - rc.top, 64));

		if (new_view_size != view_size) {
			view_size				= new_view_size;
			g_mouse_capture_state	= mouse_capture::LOST;

			//debug("resizing swap chain: %ix%i", view_size.x, view_size.y);

			gpu_reset(view_size);
			frame_init_view(view_size);
		}

		// sync mouse state

		if (g_win_focus && (g_request_mouse_capture || g_win_fullscreen) && (g_win_modal == 0)) {
			if (g_mouse_capture_state != mouse_capture::CAPTURED) {
				g_mouse_capture_state = mouse_capture::CAPTURED;
				PostMessage(g_win_hwnd, WM_APP_UPDATE_MOUSE_CURSOR, 0, 0);
			}
		}
		else {
			if (g_mouse_capture_state != mouse_capture::FREE) {
				g_mouse_capture_state = mouse_capture::FREE;
				PostMessage(g_win_hwnd, WM_APP_UPDATE_MOUSE_CURSOR, 0, 0);
			}
		}

		if (old_hide_mouse != g_hide_mouse_cursor) {
			old_hide_mouse = g_hide_mouse_cursor;
			PostMessage(g_win_hwnd, WM_APP_UPDATE_MOUSE_CURSOR, 0, 0);
		}

		// update

		gpu_begin_frame();

		int dev_notify_id = g_dev_notify_id;

		if (dev_notify_id != old_notify_id) {
			old_notify_id = dev_notify_id;
			//debug("game_thread_proc: device changed - %i", old_notify_id);
			input_update(true);
		}
		else
			input_update(false);

		frame_step(to_vec2(view_size));

		gpu_present();
	}

	return 0;
}

#define HID_USAGE_PAGE_GENERIC		((USHORT) 0x01)
#define HID_USAGE_GENERIC_MOUSE		((USHORT) 0x02)
#define HID_USAGE_GENERIC_KEYBOARD	((USHORT) 0x06)

bool set_mouse_input(HWND hwnd, bool capture) {
	RAWINPUTDEVICE rid;

	rid.usUsagePage	= HID_USAGE_PAGE_GENERIC;
	rid.usUsage		= HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags		= capture ? (RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE) : RIDEV_REMOVE;
	rid.hwndTarget	= capture ? hwnd : 0;

	if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE) {
		debug("set_mouse_input(%i): FAILED", capture);
		return false;
	}

	return true;
}

vec2i mouse_msg_pos(HWND hwnd) {
	DWORD mp = GetMessagePos();
	POINT p = { (i32)(i16)LOWORD(mp), (i32)(i16)HIWORD(mp) };
	ScreenToClient(hwnd, &p);
	return { p.x, p.y };
}

int mouse_msg_button(UINT msg, WPARAM wparam) {
	switch(msg) {
		default:
		case WM_LBUTTONDOWN:	return 0;
		case WM_LBUTTONUP:		return 0;
		case WM_LBUTTONDBLCLK:	return 0;
		case WM_RBUTTONDOWN:	return 1;
		case WM_RBUTTONUP:		return 1;
		case WM_RBUTTONDBLCLK:	return 1;
		case WM_MBUTTONDOWN:	return 2;
		case WM_MBUTTONUP:		return 2;
		case WM_MBUTTONDBLCLK:	return 2;
		case WM_XBUTTONDOWN:	return GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? 3 : 4;
		case WM_XBUTTONUP:		return GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? 3 : 4;
		case WM_XBUTTONDBLCLK:	return GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? 3 : 4;
	}
}

bool mouse_msg_down(UINT msg) {
	switch(msg) {
		default:
		case WM_LBUTTONDOWN:	return true;
		case WM_LBUTTONUP:		return false;
		case WM_LBUTTONDBLCLK:	return true;
		case WM_RBUTTONDOWN:	return true;
		case WM_RBUTTONUP:		return false;
		case WM_RBUTTONDBLCLK:	return true;
		case WM_MBUTTONDOWN:	return true;
		case WM_MBUTTONUP:		return false;
		case WM_MBUTTONDBLCLK:	return true;
		case WM_XBUTTONDOWN:	return true;
		case WM_XBUTTONUP:		return false;
		case WM_XBUTTONDBLCLK:	return true;
	}
}

void toggle_fullscreen() {
	g_win_fullscreen = !g_win_fullscreen;

	if (g_win_fullscreen) {
		GetWindowPlacement(g_win_hwnd, &g_win_old_placement);

		SetWindowLongPtr(g_win_hwnd, GWL_EXSTYLE, 0);
		SetWindowLongPtr(g_win_hwnd, GWL_STYLE, WS_OVERLAPPED | WS_VISIBLE);

		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(g_win_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);

		SetWindowPos(g_win_hwnd, 0,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_SHOWWINDOW
			);
	}
	else {
		SetWindowLongPtr(g_win_hwnd, GWL_EXSTYLE, WS_EX_OVERLAPPEDWINDOW);
		SetWindowLongPtr(g_win_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(g_win_hwnd, &g_win_old_placement);
	}
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		// lifetime

		case WM_CREATE: {
			GUID usb_serial_host = { 0x25DBCE51, 0x6C8F, 0x4A72, 0x8A, 0x6D, 0xB5, 0x4C, 0x2B, 0x4F, 0xC8, 0x35 };

			DEV_BROADCAST_DEVICEINTERFACE nf = { };

			nf.dbcc_size		= sizeof(DEV_BROADCAST_DEVICEINTERFACE);
			nf.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
			nf.dbcc_classguid	= usb_serial_host;

			if ((g_dev_notify = RegisterDeviceNotification(hwnd, &nf, DEVICE_NOTIFY_WINDOW_HANDLE)) == 0) {
				debug("RegisterDeviceNotification: failed");
			}
		}
		break;

		case WM_DESTROY:
			if (g_dev_notify != 0) {
				UnregisterDeviceNotification(g_dev_notify);
				g_dev_notify = 0;
			}
		break;

		// window interaction

		case WM_CLOSE:
			g_win_quit = true;
		return 0;

		case WM_ACTIVATE:
			if (!(g_win_focus = wparam != WA_INACTIVE)) {
				input_lost_focus();
				g_win_modal = 0;
			}
		break;

		case WM_ENTERMENULOOP: g_win_modal |= 1;  break;
		case WM_EXITMENULOOP:  g_win_modal &= ~1; break;
		case WM_ENTERSIZEMOVE: g_win_modal |= 2;  break;
		case WM_EXITSIZEMOVE:  g_win_modal &= ~2; break;

		// keys

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN: {
			bool send_key_to_game = true;

			if ((lparam & 0x2000'0000) && (wparam == VK_SPACE))
				send_key_to_game = false;

			int vk = (int)wparam;
			int sc = (lparam >> 16) & 0xFF;
			int ex = !!(lparam & (1 << 24));

			if (vk == VK_SHIFT)
				vk = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);

			switch(vk) {
				case VK_CONTROL:	vk = ex ? VK_RCONTROL	: VK_LCONTROL;	break;
				case VK_MENU:		vk = ex ? VK_RMENU		: VK_LMENU;		break;
				case VK_RETURN:		vk = ex ? VK_SEPARATOR	: VK_RETURN;	break;
				case VK_DELETE:		vk = ex ? VK_DELETE		: VK_DECIMAL;	break;
				case VK_INSERT:		vk = ex ? VK_INSERT		: VK_NUMPAD0;	break;
				case VK_END:		vk = ex ? VK_END		: VK_NUMPAD1;	break;
				case VK_DOWN:		vk = ex ? VK_DOWN		: VK_NUMPAD2;	break;
				case VK_NEXT:		vk = ex ? VK_NEXT		: VK_NUMPAD3;	break;
				case VK_LEFT:		vk = ex ? VK_LEFT		: VK_NUMPAD4;	break;
				case VK_CLEAR:		vk = ex ? VK_CLEAR		: VK_NUMPAD5;	break;
				case VK_RIGHT:		vk = ex ? VK_RIGHT		: VK_NUMPAD6;	break;
				case VK_HOME:		vk = ex ? VK_HOME		: VK_NUMPAD7;	break;
				case VK_UP:			vk = ex ? VK_UP			: VK_NUMPAD8;	break;
				case VK_PRIOR:		vk = ex ? VK_PRIOR		: VK_NUMPAD9;	break;
			}

			if (send_key_to_game)
				input_key_event(vk, !(lparam & 0x80000000));

			if (!(lparam & 0xC0000000)) {
				if (((wparam == 'F') && (GetKeyState(VK_CONTROL) & 0x8000)) ||
					((wparam == VK_RETURN) && (lparam & 0x20000000)) ||
					(wparam == VK_F11)
				) {
					//gpu_toggle_fullscreen();
					toggle_fullscreen();
				}

				if ((wparam == VK_F4) && (lparam & 0x20000000)) {
					g_win_quit = true;
				}
			}
		}
		return 0;

		case WM_CHAR:
		case WM_DEADCHAR:
		case WM_SYSDEADCHAR:
		return 0;

		case WM_SYSCHAR:
			if ((wparam == VK_SPACE) && (lparam & 0x20000000))
				break;
		return 0;

		// custom

		case WM_APP_UPDATE_MOUSE_CURSOR: {
			if (g_mouse_capture_state == mouse_capture::CAPTURED) {
				RECT rc;
				GetClientRect(g_win_hwnd, &rc);
				MapWindowPoints(g_win_hwnd, 0, (POINT*)&rc, 2);

				ClipCursor(&rc);
				SetCursor(0);
				set_mouse_input(hwnd, true);
			}
			else {
				set_mouse_input(hwnd, false);
				ClipCursor(0);
				SetCursor(LoadCursor(0, IDC_ARROW));

				RECT rc;
				GetClientRect(g_win_hwnd, &rc);
				MapWindowPoints(g_win_hwnd, 0, (POINT*)&rc, 2);
				SetCursorPos((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);
			}
		}
		return 0;

		// mouse

		case WM_SETCURSOR:
			if (LOWORD(lparam) == HTCLIENT) {
				if ((g_mouse_capture_state == mouse_capture::CAPTURED) || g_hide_mouse_cursor)
					SetCursor(0);
				else
					SetCursor(LoadCursor(0, IDC_ARROW));
				return TRUE;
			}
		break;

		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK: {
			if (!g_input.mouse_buttons)
				SetCapture(hwnd);

			input_mouse_move_event(mouse_msg_pos(hwnd));
			input_mouse_button_event(mouse_msg_button(msg, wparam), mouse_msg_down(msg));

			if (!g_input.mouse_buttons)
				ReleaseCapture();
		}
		return 0;

		case WM_MOUSEMOVE:
			input_mouse_move_event(mouse_msg_pos(hwnd));
		return 0;

		case WM_INPUT:
		{
			char buf[512];
			UINT dwSize = sizeof(buf);

			if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, buf, &dwSize, sizeof(RAWINPUTHEADER)) > 0) {
				RAWINPUT* ri = (RAWINPUT*)buf;

				if (ri->header.dwType == RIM_TYPEMOUSE) {
					if (ri->data.mouse.lLastX || ri->data.mouse.lLastY) input_mouse_relative_move_event(vec2i(ri->data.mouse.lLastX, ri->data.mouse.lLastY));

					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) input_mouse_button_event(0, true);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) input_mouse_button_event(1, true);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) input_mouse_button_event(2, true);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) input_mouse_button_event(3, true);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) input_mouse_button_event(4, true);

					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) input_mouse_button_event(0, false);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) input_mouse_button_event(1, false);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP) input_mouse_button_event(2, false);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) input_mouse_button_event(3, false);
					if (ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) input_mouse_button_event(4, false);

					//if (ri->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) input_mouse_wheel_event(-(i16)ri->data.mouse.usButtonData / WHEEL_DELTA);
				}
			}
		}
		break;

		// paint

		case WM_ERASEBKGND:
		return 1;

		case WM_PAINT:
			ValidateRect(hwnd, 0);
		return 0;

		// device change

		case WM_DEVICECHANGE:
			g_dev_notify_id++;
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// window

	WNDCLASSEX wc = { sizeof(wc) };

	wc.lpszClassName	= L"MainWnd";
	wc.lpfnWndProc		= window_proc;
	wc.hCursor			= LoadCursor(0, IDC_ARROW);

	RegisterClassEx(&wc);

	DWORD	style	= WS_OVERLAPPEDWINDOW;
	DWORD	styleEx = WS_EX_OVERLAPPEDWINDOW;
	RECT	rcWin	= { 0, 0, DEFAULT_VIEW_SIZE.x, DEFAULT_VIEW_SIZE.y };

	AdjustWindowRectEx(&rcWin, style, FALSE, styleEx);

	RECT rcDesk;
	GetClientRect(GetDesktopWindow(), &rcDesk);

	OffsetRect(&rcWin, ((rcDesk.right - rcDesk.left) - (rcWin.right - rcWin.left)) / 2, ((rcDesk.bottom - rcDesk.top) - (rcWin.bottom - rcWin.top)) / 2);

	g_win_hwnd = CreateWindowEx(styleEx, wc.lpszClassName, g_win_name, style, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, 0, 0, 0, 0);

	// init

	gpu_init(g_win_hwnd, DEFAULT_VIEW_SIZE);
	audio_init(128, 1024);
	input_init();
	frame_init();

	HANDLE game_thread = CreateThread(0, 0, game_thread_proc, 0, 0, 0);
	
	ShowWindow(g_win_hwnd, SW_SHOWNORMAL);

	// run

	for(;;) {
		DWORD r = MsgWaitForMultipleObjects(1, &game_thread, FALSE, INFINITE, QS_ALLINPUT);

		for(MSG msg; PeekMessage(&msg, 0, 0, 0, PM_REMOVE); ) {
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (r == WAIT_OBJECT_0)
			break;
	}

	// shutdown

	ShowWindow(g_win_hwnd, SW_HIDE);
	CloseHandle(game_thread);

	audio_shutdown();
	gpu_shutdown();
	DestroyWindow(g_win_hwnd);

	return 0;
}