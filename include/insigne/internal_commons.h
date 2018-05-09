#pragma once

namespace insigne {
namespace renderer {
	struct material_template_t {
		floral::inplace_array<floral::crc_string, 8u>	float_param_ids;
		floral::inplace_array<floral::crc_string, 4u>	texture2d_param_ids;
	};
}
}

