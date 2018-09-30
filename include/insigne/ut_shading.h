#pragma once

// ut = user thread

#include <floral.h>

#include "commons.h"

namespace insigne {

shader_desc_t									create_shader_desc();
const shader_handle_t							create_shader(const shader_desc_t& i_desc);

void											infuse_material(const shader_handle_t i_hdl, material_desc_t& o_mat);

void											cleanup_shading_module();

}
