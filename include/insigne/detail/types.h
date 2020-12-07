#pragma once

#include <floral.h>

#include "insigne/configs.h"
#include "insigne/commons.h"
#include "insigne/gl/identifiers.h"

namespace insigne
{
namespace detail
{

//----------------------------------------------
struct framebuffer_desc_t {
	floral::inplace_array<floral::crc_string, MAX_COLOR_ATTACHMENTS>	color_attach_ids;
	floral::inplace_array<texture_handle_t, MAX_COLOR_ATTACHMENTS>		color_attach_textures;
	floral::vec4f								clear_color;
	texture_handle_t							depth_texture;
	GLuint										gpu_handle;
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

struct vbdesc_t
{
	size										region_size;
	size										stride;
	GLuint										gpu_handle;
	u32											count;
	buffer_usage_e								usage;
};

struct ibdesc_t
{
	size										region_size;
	u32											count;
	GLuint										gpu_handle;
	buffer_usage_e								usage;
};

struct ubdesc_t
{
	size										region_size;
	size										data_size;
	size										alignment;
	GLuint										gpu_handle;
	buffer_usage_e								usage;
};

//----------------------------------------------

struct texture_desc_t
{
	s32											width, height, depth;
	texture_format_e							format;
	filtering_e									min_filter, mag_filter;
	wrap_e										wrap_s, wrap_t, wrap_r;
	texture_dimension_e							dimension;
	texture_compression_e						compression;
	GLuint										gpu_handle;
	bool										has_mipmap;
};

}
}
