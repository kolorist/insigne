#pragma once

// ut = user thread

#include <floral.h>

namespace insigne {

shader_desc_t									create_shader_desc();
const shader_handle_t							create_shader(const shader_desc_t& i_desc);

material_desc_t									create_material_desc();
const material_handle_t							create_material(const material_desc_t& i_desc);

}
