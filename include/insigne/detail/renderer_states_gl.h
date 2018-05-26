#pragma once

#include <floral.h>

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

		//floral::inplace_array<param_t, 8u>		int_params;
		floral::inplace_array<GLuint, 8u>		float_params;
		floral::inplace_array<GLuint, 8u>		vec3_params;
		floral::inplace_array<GLuint, 4u>		mat4_params;
		floral::inplace_array<GLuint, 4u>		texture2d_params;
		/*
		floral::inplace_array<param_t, 4u>		texture_cube_params;
		floral::inplace_array<param_t, 8u>		vec2_params;
		floral::inplace_array<param_t, 8u>		vec3_params;
		floral::inplace_array<param_t, 8u>		vec4_params;
		floral::inplace_array<param_t, 4u>		mat3_params;
		floral::inplace_array<param_t, 4u>		mat4_params;
		*/
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

	typedef floral::fixed_array<shader, linear_allocator_t>					shader_array_t;
	typedef floral::fixed_array<material_template_t, linear_allocator_t>	material_template_array_t;
	typedef floral::fixed_array<texture, linear_allocator_t>				texture_array_t;
	typedef floral::fixed_array<surface, linear_allocator_t>				surface_array_t;

	extern shader_array_t						s_shaders;
	extern material_template_array_t			s_material_templates;
	extern texture_array_t						s_textures;
	extern surface_array_t						s_surfaces;
}

}
}
