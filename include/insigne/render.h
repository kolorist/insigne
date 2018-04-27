#pragma once

#include <floral.h>

#include "commons.h"
#include "generated_code/proxy.h"

namespace insigne {

	void										initialize_render_thread();
	void										wait_for_initialization();

	void										begin_frame();
	void										end_frame();
	void										dispatch_frame();

	void										set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);
	void										upload_texture(texture_format_e i_format, voidptr i_data);
	const surface_handle_t						upload_surface(voidptr i_vertices, voidptr i_indices, size i_stride, const u32 i_vcount, const u32 i_icount);
	const shader_handle_t						compile_shader(const_cstr i_vertstr, const_cstr i_fragstr);

	void										draw_surface(const surface_handle_t i_surfaceHdl, const shader_handle_t i_shaderHdl);
}
