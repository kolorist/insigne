#pragma once

// ut = user thread

#include <floral.h>

#include "commons.h"

namespace insigne {

shader_desc_t									create_shader_desc();
const shader_handle_t							create_shader(const shader_desc_t& i_desc);

void											infuse_material(const shader_handle_t i_hdl, material_desc_t& o_mat);

inline const s32								get_material_texture_slot(const material_desc_t& o_mat, const_cstr i_id);
inline const s32								get_material_uniform_block_slot(const material_desc_t& o_mat, const_cstr i_id);

void											cleanup_shading_module(); // not used

ssize											get_shading_resource_state();
void											cleanup_shading_resource(const ssize i_stateId);

namespace helpers
{
void											assign_uniform_block(material_desc_t& io_mat, const_cstr i_id, const size i_offset, const size i_range, const ub_handle_t i_ub);
void											assign_texture(material_desc_t& io_mat, const_cstr i_id, const texture_handle_t i_tex);
}

}

#include "ut_shading.hpp"
