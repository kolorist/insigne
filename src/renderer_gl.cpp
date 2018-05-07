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

	// -----------------------------------------
	static GLenum s_draw_types[] = {
		GL_STATIC_DRAW,
		GL_DYNAMIC_DRAW,
		GL_STREAM_DRAW };

	static GLenum s_cmp_funcs[] = {
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_ALWAYS
	};

	static GLenum s_front_faces[] = {
		GL_CW,
		GL_CCW
	};

	static GLenum s_blend_equations[] = {
		GL_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		GL_MIN,
		GL_MAX
	};

	static GLenum s_factors[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA
	};

	static GLenum s_stencil_ops[] = {
		GL_KEEP,
		GL_ZERO,
		GL_REPLACE,
		GL_INCR,
		GL_INCR_WRAP,
		GL_DECR,
		GL_DECR_WRAP,
		GL_INVERT
	};

	static GLenum s_texture_formats[] = {
		GL_RGB,
		GL_RGB,
		GL_RGB,
		GL_RGBA,
		GL_RGBA,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_STENCIL
	};

	static GLenum s_internal_formats[] = {
		GL_RG16F,
		GL_RGB16F,
		GL_RGBA16F,
		GL_RGB8,
		GL_RGBA8,
		GL_SRGB8,
		GL_SRGB8_ALPHA8,
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH24_STENCIL8
	};

	static GLenum s_data_types[] = {
		GL_UNSIGNED_BYTE,
		GL_BYTE,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT_24_8
	};

	static GLenum s_filterings[] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	// -----------------------------------------

	void initialize_renderer()
	{
		s_shaders.init(64u, &g_persistance_allocator);
		s_textures.init(64u, &g_persistance_allocator);
		s_surfaces.init(256u, &g_persistance_allocator);
	}

	// -----------------------------------------
	template <>
	void set_depth_test<true_type>(const compare_func_e i_depthFunc)
	{
		pxEnable(GL_DEPTH_TEST);
		pxDepthFunc(s_cmp_funcs[static_cast<s32>(i_depthFunc)]);
	}

	template <>
	void set_depth_test<false_type>(const compare_func_e i_depthFunc)
	{
		pxDisable(GL_DEPTH_TEST);
	}

	template <>
	void set_depth_write<true_type>()
	{
		pxDepthMask(GL_TRUE);
	}

	template <>
	void set_depth_write<false_type>()
	{
		pxDepthMask(GL_FALSE);
	}

	template <>
	void set_cull_face<true_type>(const front_face_e i_frontFace)
	{
		pxEnable(GL_CULL_FACE);
		pxFrontFace(s_front_faces[static_cast<s32>(i_frontFace)]);
	}

	template <>
	void set_cull_face<false_type>(const front_face_e i_frontFace)
	{
		pxDisable(GL_CULL_FACE);
	}

	template <>
	void set_blending<true_type>(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor)
	{
		pxEnable(GL_BLEND);
		pxBlendEquation(s_blend_equations[static_cast<s32>(i_blendEqu)]);
		pxBlendFunc(s_factors[static_cast<s32>(i_sfactor)], s_factors[static_cast<s32>(i_dfactor)]);
	}

	template <>
	void set_blending<false_type>(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor)
	{
		pxDisable(GL_BLEND);
	}

	template <>
	void set_scissor_test<true_type>(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
	{
		pxEnable(GL_SCISSOR_TEST);
		pxScissor(i_x, i_y, i_width, i_height);
	}

	template <>
	void set_scissor_test<false_type>(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
	{
		pxDisable(GL_SCISSOR_TEST);
	}

	template <>
	void set_stencil_test<true_type>(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass)
	{
		pxEnable(GL_STENCIL_TEST);
		pxStencilFunc(s_cmp_funcs[static_cast<s32>(i_func)], i_ref, i_mask);
		pxStencilOp(s_stencil_ops[static_cast<s32>(i_sfail)], s_stencil_ops[static_cast<s32>(i_dpfail)], s_stencil_ops[static_cast<s32>(i_dppass)]);
	}

	template <>
	void set_stencil_test<false_type>(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass)
	{
		pxDisable(GL_STENCIL_TEST);
	}
	// -----------------------------------------

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

	texture_handle_t create_texture()
	{
		u32 idx = s_textures.get_size();
		s_textures.push_back(texture());
		return static_cast<texture_handle_t>(idx);
	}

	// we should gather all standalone 2d textures and upload them in one go
	void upload_texture2d(const texture_handle_t& i_hdl,
			const s32 i_width, const s32 i_height,
			const texture_format_e i_format, const texture_internal_format_e i_internalFormat,
			const data_type_e i_dataType, voidptr i_data,
			const filtering_e i_minFil /* = filtering_e::nearest */, const filtering_e i_magFil /* = filtering_e::nearest */)
	{
		texture& thisTexture = s_textures[i_hdl];
		GLuint newTexture = 0;
		
		pxGenTextures(1, &newTexture);
		pxBindTexture(GL_TEXTURE_2D, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_filterings[static_cast<s32>(i_magFil)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_filterings[static_cast<s32>(i_minFil)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		pxTexImage2D(GL_TEXTURE_2D, 0,
				s_internal_formats[static_cast<s32>(i_internalFormat)], i_width, i_height, 0,
				s_texture_formats[static_cast<s32>(i_format)], s_data_types[static_cast<s32>(i_dataType)], (GLvoid*)i_data);

		pxBindTexture(GL_TEXTURE_2D, 0);
		thisTexture.gpu_handle = newTexture;
		thisTexture.width = i_width;
		thisTexture.height = i_height;
		thisTexture.format = s_texture_formats[static_cast<s32>(i_format)];
		thisTexture.internal_format = s_internal_formats[static_cast<s32>(i_internalFormat)];
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
