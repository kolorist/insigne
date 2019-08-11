#pragma once

#include <floral.h>

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

namespace insigne
{
namespace detail
{

typedef floral::fixed_array<texture_desc_t, linear_allocator_t>	textures_pool_t;
extern textures_pool_t							g_textures_pool;

const texture_handle_t							create_texture();	/* ut */
const texture_handle_t							get_last_texture();	/* ut */
void											upload_texture(const texture_handle_t i_hdl, const insigne::texture_desc_t& i_uploadDesc);

// ---------------------------------------------
void											initialize_textures_module();
void											cleanup_textures_module();
void											process_textures_command_buffer(const size i_cmdBuffId);

}
}
