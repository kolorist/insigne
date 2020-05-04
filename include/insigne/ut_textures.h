#pragma once

#include <floral.h>

#include "commons.h"

namespace insigne
{

const size										calculate_texture_memsize(const texture_desc_t& i_desc);
const size										prepare_texture_desc(texture_desc_t& io_desc);
const texture_handle_t							create_texture(const texture_desc_t& i_desc);

void											update_texture(const texture_handle_t i_hdl, const texture_desc_t& i_desc);
void											copy_update_texture(const texture_handle_t i_hdl, voidptr i_data, const size i_dataSize);

void											cleanup_textures_module();

ssize											get_textures_resource_state();
void											cleanup_textures_resource(const ssize i_stateId);

}
