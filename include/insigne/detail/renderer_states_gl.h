#pragma once

#include <floral.h>

#include "insigne/configs.h"
#include "insigne/commons.h"
#include "insigne/internal_commons.h"
#include "insigne/memory.h"
#include "insigne/gl/identifiers.h"

namespace insigne {
namespace renderer {

namespace detail {

struct param_t {
	floral::crc_string						name;
	GLuint									id;

	param_t() { }
	param_t(const_cstr i_name, const GLuint i_id)
		: name(i_name)
		, id(i_id)
	{ }
};

struct shader {
	GLuint									gpu_handle;

	floral::inplace_array<GLuint, MAX_FLOAT_UNIFORMS>		float_params;
	floral::inplace_array<GLuint, MAX_VEC3_UNIFORMS>		vec3_params;
	floral::inplace_array<GLuint, MAX_MAT4_UNIFORMS>		mat4_params;
	floral::inplace_array<GLuint, MAX_TEX2D_UNIFORMS>		texture2d_params;
	floral::inplace_array<GLint, MAX_TEXCUBE_UNIFORMS>		texture_cube_params;
	floral::inplace_array<GLuint, MAX_UNIFORM_BLOCKS>		uniform_block_params;
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

struct framebuffer {
	GLuint									gpu_handle;
	s32										width, height;
	s32										color_attachments_count;
	texture_handle_t						color_attachments[4];
	texture_handle_t						depthstencil_attachment;
};

typedef floral::fixed_array<shader, linear_allocator_t>					shader_array_t;
typedef floral::fixed_array<material_template_t, linear_allocator_t>	material_template_array_t;
typedef floral::fixed_array<texture, linear_allocator_t>				texture_array_t;
typedef floral::fixed_array<surface, linear_allocator_t>				surface_array_t;
typedef floral::fixed_array<framebuffer, linear_allocator_t>			framebuffer_array_t;

extern shader_array_t						s_shaders;
extern material_template_array_t			s_material_templates;
extern texture_array_t						s_textures;
extern surface_array_t						s_surfaces;
extern framebuffer_array_t					s_framebuffers;

}

}
}
