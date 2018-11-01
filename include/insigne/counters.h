#pragma once

#include <floral.h>
#include <atomic>

namespace insigne {

struct global_counters {
	std::atomic<u64>							current_frame_idx;
	u64											current_render_frame_idx;		// actually, this is the current FBO frame idx
	u64											current_submit_frame_idx;
};

struct debug_global_counters {
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

struct debug_frame_counters {
	u32											num_init_commands;
	u32											num_render_commands;
	u32											num_load_commands;
	u32											num_state_commands;
};


// ---------------------------------------------
extern global_counters							g_global_counters;
extern debug_global_counters					g_debug_global_counters;
extern debug_frame_counters						g_debug_frame_counters;

// ---------------------------------------------
const u64										get_current_frame_idx();

}
