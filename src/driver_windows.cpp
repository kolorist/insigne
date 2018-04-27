#include "insigne/driver.h"

#include <platform/windows/system.h>

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/oglapis.h"

#include <Windows.h>

namespace insigne {

	static HMODULE								s_ogl_module;

	typedef HGLRC								(_stdcall *wglCreateContextAttribsARB_t)(HDC i_hDC, HGLRC i_hShareContext, const int* i_attribList);
	static wglCreateContextAttribsARB_t			wglCreateContextAttribsARB;

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
#		include "insigne/generated_code/helpers.h"
#undef	API_HELPER
		wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)get_api_address("wglCreateContextAttribsARB");
	}

	void initialize_driver()
	{
		g_gl_context.hwnd = calyx::g_windows_context_attribs.hwnd;
		HDC hDC = GetDC(g_gl_context.hwnd);

		PIXELFORMATDESCRIPTOR pxFormatDesc;
		memset(&pxFormatDesc, 0, sizeof(pxFormatDesc));
		pxFormatDesc.nSize = sizeof(pxFormatDesc);
		pxFormatDesc.nVersion = 1;
		pxFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
		pxFormatDesc.iPixelType = PFD_TYPE_RGBA;
		pxFormatDesc.cColorBits = 32;
		pxFormatDesc.cDepthBits = 24;
		pxFormatDesc.cStencilBits = 8;

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

		/*
		const GLubyte* verStr = OpenGLDriver::pfGetString(GL_VERSION);
		const GLubyte* glslStr = OpenGLDriver::pfGetString(GL_SHADING_LANGUAGE_VERSION);
		const GLubyte* vendorStr = OpenGLDriver::pfGetString(GL_VENDOR);
		const GLubyte* rendererStr = OpenGLDriver::pfGetString(GL_RENDERER);

		CLOVER_VERBOSE("OGL information:			\
			\nOpenGL version: %s				\
			\nGLSL version: %s					\
			\nVendor: %s						\
			\nRenderer: %s", verStr, glslStr, vendorStr, rendererStr);

		GLint numExtension = 0;
		OpenGLDriver::pfGetIntegerv(GL_NUM_EXTENSIONS, &numExtension);
		CLOVER_VERBOSE("Number of extensions: %d", numExtension);
		for (s32 i = 0; i < numExtension; i++) {
			const GLubyte* extStr = OpenGLDriver::pfGetStringi(GL_EXTENSIONS, i);
			CLOVER_VERBOSE("Ext %d: %s", i, extStr);
		}

		driver::g_GraphicsDriverStates.pm_VSyncEnable = true;
		driver::Initialize();
		*/
	}

	void create_main_context()
	{
		floral::lock_guard guard(g_gl_context.init_mtx);
		int attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 5,
			GLX_CONTEXT_FLAGS_ARB, 0,
			0
		};
		g_gl_context.main_context = wglCreateContextAttribsARB(g_gl_context.dc, 0, attribs);
		wglMakeCurrent(g_gl_context.dc, g_gl_context.main_context);
	}

	void create_shared_context()
	{
		floral::lock_guard guard(g_gl_context.init_mtx);
		int attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 5,
			GLX_CONTEXT_FLAGS_ARB, 0,
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
