#include "insigne/driver.h"

#include <clover.h>

#include <calyx/platform/android/system.h>

#include "insigne/configs.h"
#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/oglapis.h"
#include "insigne/internal_states.h"

#include <android/native_window_jni.h>

namespace insigne {

static ANativeWindow* s_cachedNativeWindow = nullptr;
static EGLint s_format;

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
	using namespace calyx::platform::android;

	android_context_attribs* androidCtx = get_android_context_attribs();
	context_attribs* commonCtx = get_context_attribs();
	system_info_t* systemInfo = get_system_info();

	s_cachedNativeWindow = androidCtx->native_window;

	systemInfo->primary_screen_width = ANativeWindow_getWidth(s_cachedNativeWindow);
	systemInfo->primary_screen_height = ANativeWindow_getHeight(s_cachedNativeWindow);

	// informations
	CLOVER_INFO("system_info.primary_screen_width: %d", systemInfo->primary_screen_width);
	CLOVER_INFO("system_info.primary_screen_height: %d", systemInfo->primary_screen_height);
	if (commonCtx->window_scale > 0.0f)
	{
		commonCtx->window_width = (u32)((f32)systemInfo->primary_screen_width * commonCtx->window_scale);
		commonCtx->window_height = (u32)((f32)systemInfo->primary_screen_height * commonCtx->window_scale);
		CLOVER_INFO("Render resolution (scale = %4.2f): %d x %d",
				commonCtx->window_scale, commonCtx->window_width, commonCtx->window_height);
	}
	else
	{
		CLOVER_INFO("Render resolution: %d x %d", commonCtx->window_width, commonCtx->window_height);
	}

	// update settings
	g_settings.native_res_x = commonCtx->window_width;
	g_settings.native_res_y = commonCtx->window_height;

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

	//EGLint format;
	EGLint numConfigs;

	androidCtx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(androidCtx->display, 0, 0);

	eglChooseConfig(androidCtx->display, attribs, &androidCtx->config, 1, &numConfigs);
	eglGetConfigAttrib(androidCtx->display, androidCtx->config, EGL_NATIVE_VISUAL_ID, &s_format);

	ANativeWindow_setBuffersGeometry(
			s_cachedNativeWindow,
			commonCtx->window_width,
			commonCtx->window_height,
			s_format);

	androidCtx->surface = eglCreateWindowSurface(androidCtx->display,
			androidCtx->config, androidCtx->native_window, 0);
	androidCtx->main_context = eglCreateContext(androidCtx->display, androidCtx->config,
			EGL_NO_CONTEXT, ctAttribs);
	eglMakeCurrent(androidCtx->display,
			androidCtx->surface,
			androidCtx->surface,
			androidCtx->main_context);

	EGLint width = 0, height = 0;
	eglQuerySurface(androidCtx->display, androidCtx->surface, EGL_WIDTH, &width);
	eglQuerySurface(androidCtx->display, androidCtx->surface, EGL_HEIGHT, &height);

	gl_debug_info& debugInfo = get_driver_info();
	memset(&debugInfo, 0, sizeof(gl_debug_info));

	const GLubyte* verStr = pxGetString(GL_VERSION);
	const GLubyte* glslStr = pxGetString(GL_SHADING_LANGUAGE_VERSION);
	const GLubyte* vendorStr = pxGetString(GL_VENDOR);
	const GLubyte* rendererStr = pxGetString(GL_RENDERER);

	strcpy(debugInfo.renderer_name, (const char*)rendererStr);
	strcpy(debugInfo.vendor_name, (const char*)vendorStr);
	strcpy(debugInfo.ogl_version, (const char*)verStr);
	strcpy(debugInfo.glsl_version, (const char*)glslStr);

	CLOVER_VERBOSE("OGL information:			\
		\n\tOpenGL version: %s				\
		\n\tGLSL version: %s					\
		\n\tVendor: %s						\
		\n\tRenderer: %s						\
		\n\tResolution: %d x %d",
		verStr, glslStr, vendorStr, rendererStr,
		width, height);

	GLint numExtension = 0;
	pxGetIntegerv(GL_NUM_EXTENSIONS, &numExtension);
	CLOVER_VERBOSE("Number of extensions: %d", numExtension);
	debugInfo.num_extensions = numExtension;

	for (s32 i = 0; i < numExtension; i++)
	{
		const GLubyte* extStr = pxGetStringi(GL_EXTENSIONS, i);
		CLOVER_VERBOSE("Ext %d: %s", i, extStr);

		strcpy(debugInfo.extensions[i], (const char*)extStr);
	}

	CLOVER_VERBOSE("Color-renderable texture formats:");
	for (size i = 0; i < numExtension; i++)
	{
		if (strstr(debugInfo.extensions[i], "_color_buffer_float") != nullptr)
		{
			CLOVER_VERBOSE("Found '_color_buffer_float' support");
			CLOVER_VERBOSE(" > [cr] GL_RGBA16F");
			CLOVER_VERBOSE(" > [cr] GL_R11F_G11F_B10F");
			CLOVER_VERBOSE(" > [tex] GL_RG16F");
		}
		else if (strstr(debugInfo.extensions[i], "_color_buffer_half_float") != nullptr)
		{
			CLOVER_VERBOSE("Found '_color_buffer_half_float' support");
			CLOVER_VERBOSE(" > [cr] GL_RGB16F");
		}
		else if (strstr(debugInfo.extensions[i], "_texture_rg") != nullptr)
		{
			CLOVER_VERBOSE("Found '_texture_rg' support");
			CLOVER_VERBOSE(" > [tex] GL_RG8");
		}
	}

	GLint ubOffsetAlignment = 0;
	GLint ubMaxBinding = 0;
	GLint ubMaxBlockSize = 0;
	pxGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubOffsetAlignment);
	pxGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &ubMaxBinding);
	pxGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &ubMaxBlockSize);
	CLOVER_VERBOSE("Uniform Buffer Specifications:\
			\n\tOffset Alignment: %d\
			\n\tMax Bindings Count: %d\
			\n\tMax Uniform Block Size: %d",
			ubOffsetAlignment, ubMaxBinding, ubMaxBlockSize);

	g_gpu_capacities.ub_max_size = ubMaxBlockSize;
	g_gpu_capacities.ub_desired_offset = ubOffsetAlignment;
}

void refresh_context()
{
	using namespace calyx;
	using namespace calyx::platform::android;
	android_context_attribs* androidCtx = get_android_context_attribs();
	context_attribs* commonCtx = get_context_attribs();

	if (!androidCtx->native_window) {
		return;
	}

	if (s_cachedNativeWindow == androidCtx->native_window)
	{
		return;
	}
	s_cachedNativeWindow = androidCtx->native_window;

	// destroy the old one
	eglMakeCurrent(androidCtx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(androidCtx->display, androidCtx->surface);
	androidCtx->surface = EGL_NO_SURFACE;

	ANativeWindow_setBuffersGeometry(
			s_cachedNativeWindow,
			commonCtx->window_width,
			commonCtx->window_height,
			s_format);

	// create a new one
	androidCtx->surface = eglCreateWindowSurface(androidCtx->display,
			androidCtx->config, androidCtx->native_window, 0);

	eglMakeCurrent(androidCtx->display,
			androidCtx->surface,
			androidCtx->surface,
			androidCtx->main_context);

	CLOVER_VERBOSE("Finished refreshing GL Surface.");
}

void create_shared_context()
{
}

void swap_buffers()
{
	using namespace calyx::platform::android;
	android_context_attribs* androidCtx = get_android_context_attribs();

	eglSwapBuffers(androidCtx->display, androidCtx->surface);
}

}
