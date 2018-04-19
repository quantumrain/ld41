#include "pch.h"
#include "base.h"
#include "sys.h"

#define DEBUG_LOG_COLS 101
#define DEBUG_LOG_ROWS 45

enum debug_line_type {
	DEBUG_LINE,
	DEBUG_CIRCLE,
	DEBUG_RECT,
	DEBUG_FILLED_RECT
};

struct debug_line_data {
	debug_line_type type;
	vec2 a, b;
	rgba colour;
	int duration_in_frames;
};

draw_list g_debug_draw_2d;
draw_list g_debug_draw_3d;
char g_debug_log[DEBUG_LOG_ROWS][DEBUG_LOG_COLS];
rgba g_debug_log_colours[DEBUG_LOG_ROWS];
int g_debug_log_time;

array<debug_line_data> g_debug_line_data;
mat44 g_debug_proj_view;

void debug_init() {
	g_debug_draw_2d.init(64 * 1024, 4096);
	g_debug_draw_3d.init(64 * 1024, 4096);
}

// debug log

void debug_log(rgba c, const char* fmt, ...) {
	memmove(g_debug_log[0], g_debug_log[1], (DEBUG_LOG_ROWS - 1) * DEBUG_LOG_COLS * sizeof(char));
	memmove(&g_debug_log_colours[0], &g_debug_log_colours[1], (DEBUG_LOG_ROWS - 1) * sizeof(rgba));

	va_list ap;
	va_start(ap, fmt);
	_vsnprintf_s(g_debug_log[DEBUG_LOG_ROWS - 1], DEBUG_LOG_COLS - 1, _TRUNCATE, fmt, ap);
	va_end(ap);

	g_debug_log_colours[DEBUG_LOG_ROWS - 1] = c;
	g_debug_log_time = 0;
}

void debug_text(vec2 p, rgba c, const char* fmt, ...) {
	char buf[512];

	va_list ap;
	va_start(ap, fmt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
	va_end(ap);

	draw_string(g_debug_draw_2d, p, vec2(1.0f), 0, c, buf);
}

// debug lines

void debug_line(vec2 a, vec2 b, rgba colour, int duration_in_frames) {
	g_debug_line_data.push_back({ DEBUG_LINE, a, b, colour, duration_in_frames });
}

void debug_circle(vec2 p, float radius, rgba colour, int duration_in_frames) {
	g_debug_line_data.push_back({ DEBUG_CIRCLE, p, radius, colour, duration_in_frames });
}

void debug_rect(vec2 a, vec2 b, rgba colour, int duration_in_frames) {
	g_debug_line_data.push_back({ DEBUG_RECT, a, b, colour, duration_in_frames });
}

void debug_filled_rect(vec2 a, vec2 b, rgba colour, int duration_in_frames) {
	g_debug_line_data.push_back({ DEBUG_FILLED_RECT, a, b, colour, duration_in_frames });
}

void debug_step() {
	for(int i = 0; i < g_debug_line_data.size(); ) {
		debug_line_data* d = &g_debug_line_data[i];

		if (--d->duration_in_frames <= 0) {
			g_debug_line_data.swap_remove(i);
			continue;
		}

		i++;
	}
}

void debug_set_proj_view(const mat44& proj_view) {
	g_debug_proj_view = proj_view;
}

// render debug

void debug_render(vec2 view_size) {
	// debug lines

	{
		draw_context dc(g_debug_draw_3d);

		for(auto& d : g_debug_line_data) {
			switch(d.type) {
				case DEBUG_LINE:
					dc.line(d.a, d.b, 0.25f, d.colour);
				break;

				case DEBUG_CIRCLE:
					dc.shape_outline(d.a, 32, d.b.x, 0.0f, 0.25f, d.colour);
				break;

				case DEBUG_RECT:
					dc.rect_outline(d.a, d.b, 0.25f, d.colour);
				break;

				case DEBUG_FILLED_RECT:
					dc.rect(d.a, d.b, d.colour);
				break;
			}
		}
	}

	// debug log

	if (g_debug_log_time++ < 180) {
		draw_context dc(g_debug_draw_2d);

		for(int i = 0; i < DEBUG_LOG_ROWS; i++)
			draw_string(dc, vec2(0.0f, i * 8.0f), vec2(1.0f), 0, g_debug_log_colours[i], g_debug_log[i]);
	}

	// present stats

	{
		// timing (move this into game_thread_proc)

		const int MAX_H = 60;
		static float h[MAX_H];
		memmove(&h[0], &h[1], (MAX_H - 1) * sizeof(float));

		static u64 t_prev = timer_ticks();
		u64 t_cur = timer_ticks();
		h[MAX_H - 1] = (float)timer_ticks_to_ms(t_cur - t_prev);
		t_prev = t_cur;

		// debug toggle

		static bool debug_show_stats;

		if (is_key_pressed(KEY_F1))
			debug_show_stats = !debug_show_stats;

		// draw

		if (debug_show_stats) {
			draw_context dc(g_debug_draw_2d);

			float low	= h[0];
			float high	= h[0];
			float avg	= h[0];

			float gap	= 2.0f;
			float right	= (MAX_H - 1.0f) * gap;

			draw_context gr = dc.copy().push(translate(vec3(640.0f - right, 50.0f, 0.0f)));

			gr.line(vec2(0.0f, 0.0f), vec2(right, 0.0f), 0.25f, colours::GREEN);
			gr.line(vec2(0.0f, -1000.0f / 60.0f), vec2(right, -1000.0f / 60.0f), 0.25f, colours::GREEN * 0.75f);
			gr.line(vec2(0.0f, -1000.0f / 30.0f), vec2(right, -1000.0f / 30.0f), 0.25f, colours::RED * 0.5f);
			gr.line(vec2(0.0f, 0.0f), vec2(0.0f, -50.0f), 0.25f, colours::GREEN);
			gr.line(vec2(right, 0.0f), vec2(right, -50.0f), 0.25f, colours::GREEN);

			for(int i = 1; i < MAX_H; i++) {
				low   = min(low, h[i]);
				high  = max(high, h[i]);
				avg  += h[i];

				gr.line(vec2((i - 1) * gap, -h[i - 1]), vec2(i * gap, -h[i]), 0.25f, colours::YELLOW);
			}

			avg /= (float)MAX_H;

			draw_stringf(dc, vec2(640.0f, 51.0f), vec2(1.0f), TEXT_RIGHT, colours::SILVER, "%.1f / %.1f / %.1f", low, avg, high);
			draw_stringf(dc, vec2(640.0f, 59.0f), vec2(1.0f), TEXT_RIGHT, colours::SILVER, "%.1fms (%.0f)", h[MAX_H - 1], 1000.0 / avg);
		}
	}

	// render

	gpu_set_render_target(gpu_backbuffer(), 0);
	gpu_set_viewport(vec2(0, 0), view_size, vec2(0.0f, 1.0f));

	gpu_set_pipeline(g_pipeline_sprite);

	gpu_set_const(0, g_debug_proj_view);
	g_debug_draw_3d.render();

	gpu_set_const(0, fit_ui_proj_view(640.0f, 360.0f, view_size.x / view_size.y, -64.0f, 64.0f));
	g_debug_draw_2d.render();

	g_debug_draw_3d.reset();
	g_debug_draw_2d.reset();
}