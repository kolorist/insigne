#pragma once

#include <floral.h>

#include "memory.h"
#include "commons.h"
#include "gl/identifiers.h"

namespace insigne {

	struct shader {
		GLuint									gpu_handle;
	};

	struct surface {
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

	// -----------------------------------------
	void										initialize_renderer();

	// -----------------------------------------
	void										clear_color(const floral::vec4f& i_color);
	void										clear_framebuffer(const bool i_clearcolor, const bool i_cleardepth);
	
	shader_handle								create_shader();
	void										compile_shader(shader_handle& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr);

	surface_handle								create_surface();
	void										upload_surface(surface_handle& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount, size i_stride);
}
