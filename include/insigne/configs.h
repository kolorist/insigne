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

#define MAX_RENDER_COMMANDS_IN_BUFFER			1u << 10
#define MAX_DRAW_COMMANDS_IN_BUFFER				4u << 10
#define MAX_SHADING_COMMANDS_IN_BUFFER			2u << 10
#define MAX_BUFFERS_COMMANDS_IN_BUFFER			2u << 10
#define MAX_TEXTURES_COMMANDS_IN_BUFFER			2u << 10

#define DYNAMIC_VERTEX_BUFFER_PAGE_SIZE			2u << 20
#define DYNAMIC_INDEX_BUFFER_PAGE_SIZE			1u << 20
#define DYNAMIC_UNIFORM_BUFFER_PAGE_SIZE		1u << 20

namespace insigne
{

// -----------------------------------------
struct settings_t
{
	size										frame_shader_allocator_size_mb;
	size										frame_buffers_allocator_size_mb;
	size										frame_textures_allocator_size_mb;
	size										frame_render_allocator_size_mb;
	size										frame_draw_allocator_size_mb;
	size										draw_cmdbuff_arena_size_mb;
	size										post_draw_cmdbuff_arena_size_mb;

	ssize										surface_types_count;
	ssize										post_surface_types_count;

	u32											native_res_x;
	u32											native_res_y;
};
extern settings_t								g_settings;

}
