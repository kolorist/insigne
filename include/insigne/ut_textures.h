#pragma once

#include <floral.h>

#include "commons.h"

namespace insigne {

const size										prepare_texture_desc(texture_desc_t& io_desc);
const texture_handle_t							create_texture(const texture_desc_t& i_desc);

void											cleanup_textures_module();

}
