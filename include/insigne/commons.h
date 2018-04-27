#pragma once

namespace insigne {

	enum class texture_format {
		invalid = 0,
		rgb,
		hdr_rgb,
		srgb,
		rgba,
		hdr_rgba,
		depth,
		depth_stencil
	};

	enum class texture_internal_format {
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

	typedef u32									shader_handle;
	typedef u32									texture_handle;
	typedef u32									surface_handle;

}
