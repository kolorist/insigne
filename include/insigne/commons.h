#pragma once

namespace insigne {

	enum class render_state_togglemask_e {
		depth_test								= 1u << 0,
		depth_write								= 1u << 1,
		cull_face								= 1u << 2,
		blending								= 1u << 3,
		scissor_test							= 1u << 4,
		stencil_test							= 1u << 5
	};

	enum class render_state_valuemask_e {
		depth_func								= 0x00000007,	// bit 0, 1, 2
		front_face								= 0x00000008,	// bit 3
		blend_equation							= 0x00000030,	// bit 4, 5
		blend_func_sfactor						= 0xf0000000,	// bit 28, 29, 30, 31
		blend_func_dfactor						= 0x0f000000,	// bit 24, 25, 26, 27
		stencil_func							= 0x00700000,
		stencil_op_sfail						= 0x00070000,
		stencil_op_dpfail						= 0x00007000,
		stencil_op_dppass						= 0x00000700
	};

	typedef u32									stencil_mask_t;
	typedef s32									stencil_ref_t;
	typedef s32									scissor_pos_t;
	typedef s32									scissor_size_t;

	enum class texture_format_e {
		invalid = 0,
		rgb,
		hdr_rgb,
		srgb,
		rgba,
		hdr_rgba,
		depth,
		depth_stencil
	};

	enum class texture_internal_format_e {
		invalid = 0,
		rg16f,
		rgb16f,
		rgba16f,
		rgba8,
		srgb8,
		srgb8_alpha8,
		depth16,
		depth24,
		depth24stencil8
	};

	typedef u32									shader_handle_t;
	typedef u32									texture_handle_t;
	typedef u32									surface_handle_t;

}
