#pragma once

#include <floral.h>

namespace insigne {

namespace renderer {

	struct material_template_t {
		floral::inplace_array<floral::crc_string, 8u>	float_param_ids;
		floral::inplace_array<floral::crc_string, 8u>	vec3_param_ids;
		floral::inplace_array<floral::crc_string, 4u>	mat4_param_ids;
		floral::inplace_array<floral::crc_string, 4u>	texture2d_param_ids;
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

		param_array_t<f32, 8u>					float_params;
		param_array_t<floral::vec3f, 8u>		vec3_params;
		param_array_t<floral::mat4x4f, 4u>		mat4_params;
		param_array_t<texture_handle_t, 4u>		texture2d_params;
	};

}

