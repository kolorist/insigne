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

	enum class draw_type_e {
		static_surface = 0,
		dynamic_surface,
		stream_surface
	};

	enum class compare_func_e {
		func_never = 0,
		func_less,
		func_equal,
		func_less_or_equal,
		func_greater,
		func_not_equal,
		func_greater_or_equal,
		func_always
	};

	enum class front_face_e {
		face_cw = 0,
		face_ccw
	};

	enum class blend_equation_e {
		func_add = 0,
		func_substract,
		func_reverse_substract,
		func_min,
		func_max
	};

	enum class factor_e {
		fact_zero = 0,
		fact_one,
		fact_src_color,
		fact_one_minus_src_color,
		fact_dst_color,
		fact_one_minus_dst_color,
		fact_src_alpha,
		fact_one_minus_src_alpha,
		fact_dst_alpha,
		fact_one_minus_dst_alpha,
		fact_constant_color,
		fact_one_minus_constant_color,
		fact_constant_alpha,
		fact_one_minus_constant_alpha
	};

	enum class operation_e {
		oper_keep = 0,
		oper_zero,
		oper_replace,
		oper_increment,
		oper_increment_wrap,
		oper_decrement,
		oper_decrement_wrap,
		oper_invert
	};

	struct render_state_t {
		u32										toggles;

		compare_func_e							depth_func;

		front_face_e							front_face;

		compare_func_e							blend_equation;
		factor_e								blend_func_sfactor;
		factor_e								blend_func_dfactor;

		compare_func_e							stencil_func;
		u32										stencil_mask;
		s32										stencil_ref;
		operation_e								stencil_op_sfail;
		operation_e								stencil_op_dpfail;
		operation_e								stencil_op_dppass;

		s32										scissor_x;
		s32										scissor_y;
		s32										scissor_width;
		s32										scissor_height;
	};

	enum class texture_format_e {
		rgb = 0,
		hdr_rgb,
		srgb,
		rgba,
		hdr_rgba,
		depth,
		depth_stencil
	};

	enum class texture_internal_format_e {
		rg16f = 0,
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
