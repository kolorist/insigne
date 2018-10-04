#pragma once

#include <floral.h>

#include "insigne/configs.h"
#include "insigne/commons.h"
#include "insigne/gl/identifiers.h"

namespace insigne {
namespace detail {

//----------------------------------------------
struct framebuffer_desc_t {
	floral::inplace_array<floral::crc_string, MAX_COLOR_ATTACHMENTS>	color_attach_ids;
	floral::inplace_array<texture_handle_t, MAX_COLOR_ATTACHMENTS>		color_attach_textures;
	floral::vec4f								clear_color;
	texture_handle_t							depth_texure;
	s32											width, height;
	f32											scale;
	bool										has_depth;
};

//----------------------------------------------
struct material_template_t {
	floral::inplace_array<floral::crc_string, MAX_TEXTURE_SAMPLERS>	textures;
	floral::inplace_array<floral::crc_string, MAX_UNIFORM_BLOCKS>	uniform_blocks;
};

struct shader_slots_config_t {
	floral::inplace_array<GLint, MAX_TEXTURE_SAMPLERS>	textures;
	floral::inplace_array<GLuint, MAX_UNIFORM_BLOCKS>	uniform_blocks;
};

struct shader_desc_t {
	floral::path								vs_path, fs_path;

	GLuint										gpu_handle;
	material_template_t							material_template;
	shader_slots_config_t						slots_config;
};

//----------------------------------------------
struct vbdesc_t {
	size										region_size;
	size										stride;
	GLuint										gpu_handle;
	u32											count;
	buffer_usage_e								usage;
};

struct ibdesc_t {
	size										region_size;
	u32											count;
	GLuint										gpu_handle;
	buffer_usage_e								usage;
};

struct ubdesc_t {
	size										region_size;
	size										data_size;
	GLuint										gpu_handle;
	buffer_usage_e								usage;
};

}
}
