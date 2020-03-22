#pragma once

#include <floral.h>
#include <atomic>

namespace insigne
{

static constexpr size							debug_frames_count = 64;

struct global_counters
{
	std::atomic<u64>							current_frame_idx;
	// we won't doing modular, thus, using u64 to avoid counter wraps
	u64											current_write_slot;				// do NOT use this outside of rendering thread!!!
	std::atomic<u64>							current_read_range_begin;
	std::atomic<u64>							current_read_range_end;
	u64											current_render_frame_idx;		// actually, this is the current FBO frame idx
	u64											current_submit_frame_idx;
};

struct debug_global_counters
{
	u32											init_command_cap;
	u32											render_command_cap;
	u32											load_command_cap;
	u32											state_command_cap;

	u64											submitted_frames;
	u64											rendered_frames;

	u32											num_shaders;
	u32											shaders_cap;
	u32											num_materials;
	u32											materials_cap;
	u32											num_2d_textures;
	u32											num_cube_textures;

	u32											num_vbos;
	u32											num_vaos;
	u32											num_fbos;
};

struct debug_frame_counters
{
	u32											num_draw_commands;
	u32											num_render_commands;
	u32											num_buffer_commands;
	u32											num_texture_commands;
	u32											num_shading_commands;
	u32											empty_cmdbuffs;
	f32											frame_duration_ms;
};

struct hardware_counters
{
	f32											gpu_cycles[debug_frames_count];
	f32											fragment_cycles[debug_frames_count];
	f32											tiler_cycles[debug_frames_count];

	f32											varying_16_bits[debug_frames_count];
	f32											varying_32_bits[debug_frames_count];

	f32											external_memory_read_bytes[debug_frames_count];
	f32											external_memory_write_bytes[debug_frames_count];
};

// ---------------------------------------------

extern global_counters							g_global_counters;
extern debug_global_counters					g_debug_global_counters;
extern debug_frame_counters						g_debug_frame_counters[];
extern hardware_counters*						g_hardware_counters;

// ---------------------------------------------
const u64										get_current_frame_idx();
const u64										get_valid_debug_info_range_begin();
const u64										get_valid_debug_info_range_end();

}
