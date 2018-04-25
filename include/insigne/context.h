#pragma once

#if defined(PLATFORM_WINDOWS)
#	include <Windows.h>
#else
#endif

#include <stdaliases.h>
#include <thread/mutex.h>
#include <thread/thread.h>

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

	extern gl_context							g_gl_context;

	extern floral::thread						g_render_thread;
	extern floral::thread						g_stream_thread;
}
