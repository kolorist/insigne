#pragma once

#if defined(PLATFORM_WINDOWS)
#	include <Windows.h>
#else
#endif

#include <floral/stdaliases.h>
#include <floral/thread/mutex.h>
#include <floral/thread/thread.h>

namespace insigne {

struct gl_context {
	floral::mutex							init_mtx;
	size									geometry_streamming_budget;
	size									texture_streamming_budget;
#if defined(PLATFORM_WINDOWS)
	HWND									hwnd;
	HDC										dc;
	HGLRC									main_context;
#else
	// TODO: add
#endif
};

struct gl_debug_info {
	c8											renderer_name[256];
	c8											vendor_name[256];
	c8											ogl_version[256];
	c8											glsl_version[256];

	u32											num_extensions;
	c8											extensions[512][128];
};

extern gl_context								g_gl_context;

extern floral::thread							g_render_thread;
extern floral::thread							g_stream_thread;

}
