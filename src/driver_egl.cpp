#include "insigne/driver.h"

#include <clover.h>

#include <platform/android/system.h>

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/oglapis.h"

#include <android/native_window_jni.h>

namespace insigne {

	voidptr get_api_address(const_cstr funcName)
	{
		voidptr p = (voidptr)eglGetProcAddress(funcName);
		return p;
	}

	void initialize_apis()
	{
#define	API_HELPER(RET_TYPE, FUNC_NAME)			gl##FUNC_NAME = (gl##FUNC_NAME##_t)get_api_address("gl"#FUNC_NAME)
#		include "insigne/generated_code/helpers.h"
#undef	API_HELPER
	}

	void initialize_driver()
	{
		initialize_apis();
	}

	void create_main_context()
	{
		using namespace calyx;
		const EGLint attribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 24,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
		};

		const EGLint ctAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

		EGLint format;
		EGLint numConfigs;
		EGLConfig config;

		g_android_context_attribs.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		eglInitialize(g_android_context_attribs.display, 0, 0);

		eglChooseConfig(g_android_context_attribs.display, attribs, &config, 1, &numConfigs);
		eglGetConfigAttrib(g_android_context_attribs.display, config, EGL_NATIVE_VISUAL_ID, &format);

		ANativeWindow_setBuffersGeometry(g_android_context_attribs.native_window, 0, 0, format);

		g_android_context_attribs.surface = eglCreateWindowSurface(g_android_context_attribs.display,
				config, g_android_context_attribs.native_window, 0);
		g_android_context_attribs.main_context = eglCreateContext(g_android_context_attribs.display, config,
				EGL_NO_CONTEXT, ctAttribs);
		eglMakeCurrent(g_android_context_attribs.display,
				g_android_context_attribs.surface,
				g_android_context_attribs.surface,
				g_android_context_attribs.main_context);

		EGLint width = 0, height = 0;
		eglQuerySurface(g_android_context_attribs.display, g_android_context_attribs.surface, EGL_WIDTH, &width);
		eglQuerySurface(g_android_context_attribs.display, g_android_context_attribs.surface, EGL_HEIGHT, &height);
		CLOVER_INFO("native surface resolution: %d x %d", width, height);
	}

	void create_shared_context()
	{
	}

	void swap_buffers()
	{
		using namespace calyx;
		eglSwapBuffers(g_android_context_attribs.display, g_android_context_attribs.surface);
	}

}
