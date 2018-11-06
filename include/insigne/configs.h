#pragma once

#include <floral.h>

#define BUFFERS_COUNT							4

#define MAX_FLOAT_UNIFORMS						8u
#define MAX_VEC3_UNIFORMS						8u
#define MAX_MAT4_UNIFORMS						4u
#define MAX_TEX2D_UNIFORMS						4u
#define MAX_TEXCUBE_UNIFORMS					4u
#define MAX_UNIFORM_BLOCKS						8u
#define MAX_TEXTURE_SAMPLERS					8u

#define MAX_COLOR_ATTACHMENTS					4u

namespace insigne {

// -----------------------------------------
struct settings_t {
	u32											frame_shader_allocator_size_mb;
	u32											frame_buffers_allocator_size_mb;
	u32											frame_textures_allocator_size_mb;
	u32											frame_render_allocator_size_mb;
	u32											frame_draw_allocator_size_mb;

	u32											surface_types_count;
	u32											draw_command_buffer_size;

	u32											native_res_x;
	u32											native_res_y;
};
extern settings_t								g_settings;

}
