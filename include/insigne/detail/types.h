#pragma once

#include <floral.h>

#include "insigne/configs.h"
#include "insigne/commons.h"
#include "insigne/gl/identifiers.h"

namespace insigne {
namespace detail {

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
	GLuint										gpu_handle;
	size										region_size;
	size										stride;
	u32											count;
	buffer_usage_e								usage;
};

struct ibdesc_t {
	GLuint										gpu_handle;
	size										region_size;
	u32											count;
	buffer_usage_e								usage;
};

struct ubdesc_t {
};

}
}
