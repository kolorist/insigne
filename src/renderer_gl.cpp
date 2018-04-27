#include "insigne/renderer_gl.h"

#include <clover.h>

#include "memory.h"
#include "insigne/generated_code/proxy.h"

namespace insigne {

	static shader_array_t						s_shaders;
	static texture_array_t						s_textures;
	static surface_array_t						s_surfaces;

	void initialize_renderer()
	{
		s_shaders.init(64u, &g_persistance_allocator);
		s_textures.init(64u, &g_persistance_allocator);
	}


	void clear_color(const floral::vec4f& i_color)
	{
		pxClearColor(i_color.x, i_color.y, i_color.z, i_color.w);
	}
	void clear_framebuffer(const bool i_clearcolor, const bool i_cleardepth)
	{
		GLbitfield clearBit = 0;
		if (i_clearcolor) clearBit |= GL_COLOR_BUFFER_BIT;
		if (i_cleardepth) clearBit |= GL_DEPTH_BUFFER_BIT;
		pxClear(clearBit);
	}
	
	shader_handle create_shader()
	{
		u32 idx = s_shaders.get_size();
		s_shaders.push_back(shader());
		return static_cast<shader_handle>(idx);
	}

	void compile_shader(shader_handle& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr)
	{
		shader& thisShader = s_shaders[i_hdl];
		
		GLuint newShader = 0;
		GLuint fs = 0, vs = 0;
		newShader = pxCreateProgram();
		fs = pxCreateShader(GL_FRAGMENT_SHADER);
		vs = pxCreateShader(GL_VERTEX_SHADER);

		// compile vertex shader
		{
			GLint result = GL_FALSE;
			s32 infoLength;
			pxShaderSource(vs, 1, (const GLchar**)&i_vertstr, nullptr);
			pxCompileShader(vs);
			pxGetShaderiv(vs, GL_COMPILE_STATUS, &result);
			if (!result) {
				pxGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLength);
				cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
				pxGetShaderInfoLog(vs, infoLength + 1, NULL, shaderErrorMsg);
				CLOVER_ERROR("Vertex Shader error:\n%s", shaderErrorMsg);
				pxDeleteShader(vs);
				pxDeleteShader(fs);
				return;
			}
		}

		// compile fragment shader
		{
			GLint result = GL_FALSE;
			int infoLength;
			pxShaderSource(fs, 1, (const GLchar**)&i_fragstr, nullptr);
			pxCompileShader(fs);
			pxGetShaderiv(fs, GL_COMPILE_STATUS, &result);
			if (!result) {
				pxGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLength);
				cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
				pxGetShaderInfoLog(fs, infoLength + 1, NULL, shaderErrorMsg);
				CLOVER_ERROR("Fragment Shader error:\n%s", shaderErrorMsg);
				pxDeleteShader(vs);
				pxDeleteShader(fs);
				return;
			}
		}

		g_arena_allocator.free_all();

		// link them all!
		pxAttachShader(newShader, vs);
		pxAttachShader(newShader, fs);
		pxLinkProgram(newShader);

		// release temp shaders
		pxDeleteShader(vs);
		pxDeleteShader(fs);

		s_shaders[i_hdl].gpu_handle = newShader;
	}

	surface_handle create_surface()
	{
		u32 idx = s_surfaces.get_size();
		s_surfaces.push_back(surface());
		return static_cast<surface_handle>(idx);
	}

	void upload_surface(surface_handle& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount, size i_stride)
	{
		GLuint vbo = 0, ibo = 0;
		pxGenBuffers(1, &vbo);
		pxBindBuffer(GL_ARRAY_BUFFER, vbo);
		pxBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(i_vcount * i_stride),
			(const GLvoid*)(i_vertices),
			GL_STATIC_DRAW);
		pxBindBuffer(GL_ARRAY_BUFFER, 0);

		pxGenBuffers(1, &ibo);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		pxBufferData(GL_ELEMENT_ARRAY_BUFFER,
				(GLsizeiptr)(i_icount * sizeof(u32)),
				(const GLvoid*)(i_indices),
				GL_STATIC_DRAW);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		s_surfaces[i_hdl].vbo = vbo;
		s_surfaces[i_hdl].ibo = ibo;
	}
}
