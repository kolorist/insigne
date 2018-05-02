#pragma once

#include <floral.h>

#include "memory.h"
#include "commons.h"

namespace insigne {
namespace renderer {

	// -----------------------------------------
	void										initialize_renderer();

	// -----------------------------------------
	void										clear_color(const floral::vec4f& i_color);
	void										clear_framebuffer(const bool i_clearcolor, const bool i_cleardepth);
	
	shader_handle_t								create_shader();
	void										compile_shader(shader_handle_t& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr);

	surface_handle_t							create_surface();
	void										upload_surface(surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount, s32 i_stride, const draw_type_e i_drawType = draw_type_e::static_surface);
	void										update_surface(const surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount);
	void										draw_surface_idx(const surface_handle_t& i_surfaceHdl, const shader_handle_t& i_shaderHdl);

}
}
