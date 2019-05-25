#include "insigne/driver.h"

#include <clover.h>

#include <calyx/platform/windows/system.h>

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/oglapis.h"

#include <Windows.h>

namespace insigne {

static HMODULE								s_ogl_module;

typedef HGLRC								(_stdcall *wglCreateContextAttribsARB_t)(HDC i_hDC, HGLRC i_hShareContext, const int* i_attribList);
typedef BOOL								(_stdcall *wglSwapIntervalEXT_t)(int interval);
static wglCreateContextAttribsARB_t			wglCreateContextAttribsARB;
static wglSwapIntervalEXT_t					wglSwapIntervalEXT;

voidptr get_api_address(const_cstr funcName)
{
	voidptr p = (voidptr)wglGetProcAddress(funcName);
	if (p == 0 ||
			(p == (voidptr)0x1) || (p == (voidptr)0x2) || (p == (voidptr)0x3) ||
			(p == (voidptr)-1))
	{
		p = (voidptr)GetProcAddress(s_ogl_module, funcName);
	}

	return p;
}

void initialize_apis()
{
	s_ogl_module = LoadLibrary(TEXT("opengl32.dll"));
#define	API_HELPER(RET_TYPE, FUNC_NAME)			FLORAL_ASSERT(gl##FUNC_NAME = (gl##FUNC_NAME##_t)get_api_address("gl"#FUNC_NAME))
#	include "insigne/generated_code/helpers.h"
#undef	API_HELPER
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)get_api_address("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (wglSwapIntervalEXT_t)get_api_address("wglSwapIntervalEXT");
}

void initialize_driver()
{
	using namespace calyx::platform::windows;
	windows_context_attribs* windowsCtx = get_windows_context_attribs();

	g_gl_context.hwnd = windowsCtx->hwnd;
	HDC hDC = GetDC(g_gl_context.hwnd);

	PIXELFORMATDESCRIPTOR pxFormatDesc;
	memset(&pxFormatDesc, 0, sizeof(pxFormatDesc));
	pxFormatDesc.nSize = sizeof(pxFormatDesc);
	pxFormatDesc.nVersion = 1;
	pxFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
	pxFormatDesc.iPixelType = PFD_TYPE_RGBA;
	pxFormatDesc.cColorBits = 32;
	pxFormatDesc.cDepthBits = 32;

	int pf = ChoosePixelFormat(hDC, &pxFormatDesc);
	BOOL spxRel = SetPixelFormat(hDC, pf, &pxFormatDesc);

	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pxFormatDesc);
	ReleaseDC(g_gl_context.hwnd, hDC);

	// create OpenGL context
	g_gl_context.dc = GetDC(g_gl_context.hwnd);
	HGLRC temphGL = wglCreateContext(g_gl_context.dc);
	wglMakeCurrent(g_gl_context.dc, temphGL);

	initialize_apis();

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(temphGL);
}

void create_main_context()
{
	floral::lock_guard guard(g_gl_context.init_mtx);
	int attribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_FLAGS_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	g_gl_context.main_context = wglCreateContextAttribsARB(g_gl_context.dc, 0, attribs);
	wglMakeCurrent(g_gl_context.dc, g_gl_context.main_context);

	wglSwapIntervalEXT(1);

	gl_debug_info& debugInfo = get_driver_info();
	memset(&debugInfo, 0, sizeof(gl_debug_info));

	const GLubyte* verStr = pxGetString(GL_VERSION);
	const GLubyte* glslStr = pxGetString(GL_SHADING_LANGUAGE_VERSION);
	const GLubyte* vendorStr = pxGetString(GL_VENDOR);
	const GLubyte* rendererStr = pxGetString(GL_RENDERER);

	CLOVER_VERBOSE("OGL information:			\
		\n\tOpenGL version: %s				\
		\n\tGLSL version: %s					\
		\n\tVendor: %s						\
		\n\tRenderer: %s", verStr, glslStr, vendorStr, rendererStr);

	strcpy(debugInfo.renderer_name, (const char*)rendererStr);
	strcpy(debugInfo.vendor_name, (const char*)vendorStr);
	strcpy(debugInfo.ogl_version, (const char*)verStr);
	strcpy(debugInfo.glsl_version, (const char*)glslStr);

	GLint numExtension = 0;
	pxGetIntegerv(GL_NUM_EXTENSIONS, &numExtension);
	CLOVER_VERBOSE("Number of extensions: %d", numExtension);
	debugInfo.num_extensions = numExtension;

	for (GLint i = 0; i < numExtension; i++)
	{
		const GLubyte* extStr = pxGetStringi(GL_EXTENSIONS, i);
		CLOVER_VERBOSE("Ext %d: %s", i, extStr);

		strcpy(debugInfo.extensions[i], (const char*)extStr);
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
}

void refresh_context()
{
	using namespace calyx;
	CLOVER_VERBOSE("Finished refreshing GL Surface.");
}

void create_shared_context()
{
	floral::lock_guard guard(g_gl_context.init_mtx);
	int attribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_FLAGS_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC hGL = wglCreateContextAttribsARB(g_gl_context.dc, g_gl_context.main_context, attribs);
	wglMakeCurrent(g_gl_context.dc, hGL);
}

void swap_buffers()
{
	SwapBuffers(g_gl_context.dc);
}

}
