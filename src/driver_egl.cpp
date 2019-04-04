#include "insigne/driver.h"

#include <clover.h>

#include <platform/android/system.h>

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/oglapis.h"
#include "insigne/internal_states.h"

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

	g_android_context_attribs.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(g_android_context_attribs.display, 0, 0);

	eglChooseConfig(g_android_context_attribs.display, attribs, &g_android_context_attribs.config, 1, &numConfigs);
	eglGetConfigAttrib(g_android_context_attribs.display, g_android_context_attribs.config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(g_android_context_attribs.native_window, 1280, 720, format);

	g_android_context_attribs.surface = eglCreateWindowSurface(g_android_context_attribs.display,
			g_android_context_attribs.config, g_android_context_attribs.native_window, 0);
	g_android_context_attribs.main_context = eglCreateContext(g_android_context_attribs.display, g_android_context_attribs.config,
			EGL_NO_CONTEXT, ctAttribs);
	eglMakeCurrent(g_android_context_attribs.display,
			g_android_context_attribs.surface,
			g_android_context_attribs.surface,
			g_android_context_attribs.main_context);

	EGLint width = 0, height = 0;
	eglQuerySurface(g_android_context_attribs.display, g_android_context_attribs.surface, EGL_WIDTH, &width);
	eglQuerySurface(g_android_context_attribs.display, g_android_context_attribs.surface, EGL_HEIGHT, &height);

	gl_debug_info& debugInfo = get_driver_info();
	memset(&debugInfo, 0, sizeof(gl_debug_info));

	// informations
	CLOVER_INFO("NativeSurface resolution: %d x %d", width, height);

	const GLubyte* verStr = pxGetString(GL_VERSION);
	const GLubyte* glslStr = pxGetString(GL_SHADING_LANGUAGE_VERSION);
	const GLubyte* vendorStr = pxGetString(GL_VENDOR);
	const GLubyte* rendererStr = pxGetString(GL_RENDERER);

	strcpy(debugInfo.renderer_name, (const char*)rendererStr);
	strcpy(debugInfo.vendor_name, (const char*)vendorStr);
	strcpy(debugInfo.ogl_version, (const char*)verStr);
	strcpy(debugInfo.glsl_version, (const char*)glslStr);

	CLOVER_VERBOSE("OGL information:			\
		\nOpenGL version: %s				\
		\nGLSL version: %s					\
		\nVendor: %s						\
		\nRenderer: %s", verStr, glslStr, vendorStr, rendererStr);

	GLint numExtension = 0;
	pxGetIntegerv(GL_NUM_EXTENSIONS, &numExtension);
	CLOVER_VERBOSE("Number of extensions: %d", numExtension);
	debugInfo.num_extensions = numExtension;

	for (s32 i = 0; i < numExtension; i++) {
		const GLubyte* extStr = pxGetStringi(GL_EXTENSIONS, i);
		CLOVER_VERBOSE("Ext %d: %s", i, extStr);

		strcpy(debugInfo.extensions[i], (const char*)extStr);
	}

	detail::g_context_dirty.store(false);
}

void refresh_context()
{
	using namespace calyx;
	if (!g_android_context_attribs.native_window) {
		return;
	}

	// destroy the old one
	eglMakeCurrent(g_android_context_attribs.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(g_android_context_attribs.display, g_android_context_attribs.surface);
	g_android_context_attribs.surface = EGL_NO_SURFACE;

	// create a new one
	g_android_context_attribs.surface = eglCreateWindowSurface(g_android_context_attribs.display,
			g_android_context_attribs.config, g_android_context_attribs.native_window, 0);

	eglMakeCurrent(g_android_context_attribs.display,
			g_android_context_attribs.surface,
			g_android_context_attribs.surface,
			g_android_context_attribs.main_context);

	CLOVER_VERBOSE("Finished refreshing GL Surface.");

	detail::g_context_dirty.store(false);
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
