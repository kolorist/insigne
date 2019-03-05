#pragma once

#include <floral.h>

#include "configs.h"
#include "commons.h"

//TODO: move these to 'detail'
namespace insigne {

namespace renderer {

struct material_template_t {
	floral::inplace_array<floral::crc_string, MAX_FLOAT_UNIFORMS>	float_param_ids;
	floral::inplace_array<floral::crc_string, MAX_VEC3_UNIFORMS>	vec3_param_ids;
	floral::inplace_array<floral::crc_string, MAX_MAT4_UNIFORMS>	mat4_param_ids;
	floral::inplace_array<floral::crc_string, MAX_TEX2D_UNIFORMS>	texture2d_param_ids;
	floral::inplace_array<floral::crc_string, MAX_TEXCUBE_UNIFORMS>	texturecube_param_ids;
	floral::inplace_array<floral::crc_string, MAX_UNIFORM_BLOCKS>	uniform_block_param_ids;
};

}

template <typename t_value>
struct id_value_pair_t {
	floral::crc_string						id;
	t_value									value;
};

template <typename t_value, u32 t_capacity>
using param_array_t = floral::inplace_array<id_value_pair_t<t_value>, t_capacity>;

struct material_t {
	shader_handle_t							shader_handle;

	param_array_t<f32, MAX_FLOAT_UNIFORMS>					float_params;
	param_array_t<floral::vec3f, MAX_VEC3_UNIFORMS>			vec3_params;
	param_array_t<floral::mat4x4f, MAX_MAT4_UNIFORMS>		mat4_params;
	param_array_t<texture_handle_t, MAX_TEX2D_UNIFORMS>		texture2d_params;
	param_array_t<texture_handle_t, MAX_TEXCUBE_UNIFORMS>	texturecube_params;
	param_array_t<ub_handle_t, MAX_UNIFORM_BLOCKS>			uniform_block_params;
};

}

