#include "insigne/renderer.h"

#include <clover.h>

#include "memory.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/gl/identifiers.h"

namespace insigne {
namespace renderer {

	struct shader {
		GLuint									gpu_handle;
	};

	struct surface {
		s32										stride;
		draw_type_e								draw_type;
		u32										icount;
		GLuint									vbo;
		GLuint									ibo;
	};

	struct texture {
		GLuint									gpu_handle;
		u32										width;
		u32										height;
		GLenum									format;
		GLenum									internal_format;
	};

	typedef floral::fixed_array<shader, linear_allocator_t>		shader_array_t;
	typedef floral::fixed_array<texture, linear_allocator_t>	texture_array_t;
	typedef floral::fixed_array<surface, linear_allocator_t>	surface_array_t;

	static shader_array_t						s_shaders;
	static texture_array_t						s_textures;
	static surface_array_t						s_surfaces;

	static GLenum s_draw_types[] = {
		GL_STATIC_DRAW,
		GL_DYNAMIC_DRAW,
		GL_STREAM_DRAW };

	void initialize_renderer()
	{
		s_shaders.init(64u, &g_persistance_allocator);
		s_textures.init(64u, &g_persistance_allocator);
		s_surfaces.init(256u, &g_persistance_allocator);
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
	
	shader_handle_t create_shader()
	{
		u32 idx = s_shaders.get_size();
		s_shaders.push_back(shader());
		return static_cast<shader_handle_t>(idx);
	}

	void compile_shader(shader_handle_t& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr)
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

	surface_handle_t create_surface()
	{
		u32 idx = s_surfaces.get_size();
		s_surfaces.push_back(surface());
		return static_cast<surface_handle_t>(idx);
	}

	void upload_surface(surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount, s32 i_stride, const draw_type_e i_drawType /* = draw_type_e::static_surface */)
	{
		GLuint vbo = 0, ibo = 0;
		GLenum drawType = s_draw_types[static_cast<s32>(i_drawType)];
		pxGenBuffers(1, &vbo);
		pxBindBuffer(GL_ARRAY_BUFFER, vbo);
		pxBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(i_vcount * i_stride),
			(const GLvoid*)(i_vertices),
			drawType);
		pxBindBuffer(GL_ARRAY_BUFFER, 0);

		pxGenBuffers(1, &ibo);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		pxBufferData(GL_ELEMENT_ARRAY_BUFFER,
				(GLsizeiptr)(i_icount * sizeof(u32)),
				(const GLvoid*)(i_indices),
				drawType);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		s_surfaces[i_hdl].stride = i_stride;
		s_surfaces[i_hdl].draw_type = i_drawType;
		s_surfaces[i_hdl].icount = i_icount;
		s_surfaces[i_hdl].vbo = vbo;
		s_surfaces[i_hdl].ibo = ibo;
	}

	void update_surface(const surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount)
	{
		s32 stride = s_surfaces[i_hdl].stride;
		GLenum drawType = s_draw_types[static_cast<s32>(s_surfaces[i_hdl].draw_type)];
		GLuint vbo = s_surfaces[i_hdl].vbo;
		GLuint ibo = s_surfaces[i_hdl].ibo;

		pxBindBuffer(GL_ARRAY_BUFFER, vbo);
		pxBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(i_vcount * stride),
				(const GLvoid*)(i_vertices),
				drawType);
		pxBindBuffer(GL_ARRAY_BUFFER, 0);

		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		pxBufferData(GL_ELEMENT_ARRAY_BUFFER,
				(GLsizeiptr)(i_icount * sizeof(u32)),
				(const GLvoid*)(i_indices),
				drawType);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		s_surfaces[i_hdl].icount = i_icount;
	}

	void draw_surface_idx(const surface_handle_t& i_surfaceHdl, const shader_handle_t& i_shaderHdl)
	{
		surface surf = s_surfaces[i_surfaceHdl];
		shader shdr = s_shaders[i_shaderHdl];

		pxBindBuffer(GL_ARRAY_BUFFER, surf.vbo);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surf.ibo);
		pxEnableVertexAttribArray(0);
		pxVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, surf.stride, 0);
		pxDrawElements(GL_TRIANGLES, surf.icount, GL_UNSIGNED_INT, 0);
	}
}
}
