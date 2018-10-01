#pragma once

// ut = user thread

#include "commons.h"

namespace insigne {

const vb_handle_t								create_vb(const vbdesc_t& i_desc);
const ib_handle_t								create_ib(const ibdesc_t& i_desc);

void											update_vb(const vb_handle_t i_hdl, voidptr i_data, const u32 i_vcount, const u32 i_offsetElem);
void											update_ib(const ib_handle_t i_hdl, voidptr i_data, const u32 i_icount, const u32 i_offsetElem);

void											cleanup_buffers_module();

}