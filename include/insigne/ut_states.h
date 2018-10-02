#pragma once

#include <floral.h>

namespace insigne {

void											set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);
template <typename t_surface>
void											set_scissor_test(const bool i_enable = false, const s32 i_x = 0, const s32 i_y = 0, const s32 i_width = 0, const s32 i_height = 0);

}
