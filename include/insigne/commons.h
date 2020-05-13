#pragma once

#include <floral.h>

#include "configs.h"
#include "memory.h"

#define DEFAULT_FRAMEBUFFER_HANDLE				-1

namespace insigne
{
// ------------------------------------------------------------------

static constexpr ssize k_invalid_handle = -1;

// ------------------------------------------------------------------

enum class render_state_togglemask_e : u32
{
	depth_test								= 1u << 0,
	depth_write								= 1u << 1,
	cull_face								= 1u << 2,
	blending								= 1u << 3,
	scissor_test							= 1u << 4,
	stencil_test							= 1u << 5
};

enum class geometry_mode_e : s32
{
	points = 0,
	line_strip,
	line_loop,
	lines,
	triangle_strip,
	triangle_fan,
	triangles
};

enum class buffer_usage_e : s32
{
	static_draw = 0,
	dynamic_draw,
	stream_draw
};

enum class draw_type_e
{
	static_surface = 0,
	dynamic_surface,
	stream_surface
};

enum class compare_func_e
{
	func_never = 0,
	func_less,
	func_equal,
	func_less_or_equal,
	func_greater,
	func_not_equal,
	func_greater_or_equal,
	func_always,
	func_undefined
};

enum class front_face_e
{
	face_cw = 0,
	face_ccw,
	face_undefined
};

enum class face_side_e
{
	front_side = 0,
	back_side,
	front_and_back_side,
	undefined_side
};

enum class blend_equation_e
{
	func_add = 0,
	func_substract,
	func_reverse_substract,
	func_min,
	func_max,
	func_undefined
};

enum class polygon_mode_e {
	point = 0,
	line,
	fill
};

enum class factor_e
{
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
	fact_one_minus_constant_alpha,
	fact_undefined
};

enum class operation_e
{
	oper_keep = 0,
	oper_zero,
	oper_replace,
	oper_increment,
	oper_increment_wrap,
	oper_decrement,
	oper_decrement_wrap,
	oper_invert,
	oper_undefined
};

enum class data_type_e {
	elem_unsigned_byte = 0,
	elem_signed_byte,
	elem_unsigned_int,
	elem_signed_int,
	elem_signed_float,
	elem_unsigned_int_24_8
};

enum class cubemap_face_e {
	invalid = 0,
	positive_x,
	positive_y,
	positive_z,
	negative_x,
	negative_y,
	negative_z
};

enum class material_data_type_e {
	param_int = 0,
	param_float,
	param_sampler2d,
	param_sampler_cube,
	param_vec2,
	param_vec3,
	param_vec4,
	param_mat3,
	param_mat4,
	param_ub
};

enum class param_data_type_e
{
	param_int = 0,
	param_float,
	param_sampler2d,
	param_sampler_cube,
	param_vec2,
	param_vec3,
	param_vec4,
	param_mat3,
	param_mat4,
	param_ub
};

struct render_state_t
{
	// u8 boolean type rule:
	// 0 = false
	// 1 = true
	// 2 = undefined
	u8											depth_test			= 2;
	u8											depth_write			= 2;
	compare_func_e								depth_func			= compare_func_e::func_undefined;

	u8											cull_face			= 2;
	face_side_e									face_side			= face_side_e::undefined_side;
	front_face_e								front_face			= front_face_e::face_undefined;

	u8											blending			= 2;
	blend_equation_e							blend_equation		= blend_equation_e::func_undefined;
	factor_e									blend_func_sfactor	= factor_e::fact_undefined;
	factor_e									blend_func_dfactor	= factor_e::fact_undefined;

	u8											stencil_test		= 2;
	compare_func_e								stencil_func		= compare_func_e::func_undefined;
	u32											stencil_mask		= 0xFFFFFFFF;
	s32											stencil_ref			= 0;
	operation_e									stencil_op_sfail	= operation_e::oper_undefined;
	operation_e									stencil_op_dpfail	= operation_e::oper_undefined;
	operation_e									stencil_op_dppass	= operation_e::oper_undefined;
};

enum class texture_format_e
{
	rg = 0,
	hdr_rg,
	rgb,
	hdr_rgb,
	hdr_rgb_high,
	hdr_rgb_half,	// only use for hdr framebuffer
	srgb,
	srgba,
	rgba,
	hdr_rgba,
	depth,
	depth_stencil
};

enum class texture_dimension_e
{
	tex_2d = 0,
	tex_3d,
	tex_cube
};

enum class texture_compression_e
{
	no_compression = 0,
	dxt
};

enum class filtering_e
{
	nearest,
	linear,
	nearest_mipmap_nearest,
	linear_mipmap_nearest,
	nearest_mipmap_linear,
	linear_mipmap_linear
};

enum class wrap_e
{
	clamp_to_edge,
	mirrored_repeat,
	repeat
};

struct material_param_t {
	c8										name[128];
	material_data_type_e					data_type;

	material_param_t()
	{ }

	material_param_t(const_cstr i_name, material_data_type_e i_dataType)
	{
		strcpy(name, i_name);
		data_type = i_dataType;
	}
};

struct shader_param_t {
	c8											name[128];
	param_data_type_e							data_type;

	shader_param_t() = default;

	shader_param_t(const_cstr i_name, param_data_type_e i_dataType)
		: data_type(i_dataType)
	{
		strcpy(name, i_name);
	}
};

struct color_attachment_t {
	c8											name[128];
	texture_format_e							texture_format;
	texture_dimension_e							texture_dimension;
	s32											width, height;

	color_attachment_t()
		: texture_format(texture_format_e::rgb)
		, texture_dimension(texture_dimension_e::tex_2d)
		, width(0)
		, height(0)
	{
	}

	color_attachment_t(const_cstr i_name, const texture_format_e i_texFormat)
		: texture_format(i_texFormat)
		, texture_dimension(texture_dimension_e::tex_2d)
		, width(0)
		, height(0)
	{
		strcpy(name, i_name);
	}

	color_attachment_t(const_cstr i_name, const texture_format_e i_texFormat, const s32 i_width, const s32 i_height)
		: texture_format(i_texFormat)
		, texture_dimension(texture_dimension_e::tex_2d)
		, width(i_width)
		, height(i_height)
	{
		strcpy(name, i_name);
	}

};

using shader_param_list_t = floral::fast_fixed_array<shader_param_t, arena_allocator_t>;
using color_attachment_list_t = floral::fast_fixed_array<color_attachment_t, arena_allocator_t>;

typedef ssize									shader_handle_t;
typedef ssize									texture_handle_t;
typedef ssize									framebuffer_handle_t;
typedef ssize									color_attachment_id;
typedef ssize									vb_handle_t;
typedef ssize									ib_handle_t;
typedef ssize									ub_handle_t;
typedef s32										pb_handle_t;

// ---------------------------------------------
/* rendering callbacks
 */
typedef void									(*states_setup_func_t)();
typedef void									(*vertex_data_setup_func_t)();

// -------------------------------------------------------------------
/* pixel data alignment
 *	> for 2d texture:
 * 		pixel data is stored as a list of scanlines in a order from top to bottom of an image
 * 		mipmap is stored as a sequence of pixel data corresponding to each mip images
 *	> for cube texture:
 *		pixel data is stored in HStrip scheme
 *			positiveX, negativeX, positiveY, negativeY, posiviteZ, negativeZ
 *		mipmap is stored as a sequence of pixel data, face after face:
 *			mip0 - positiveX, mip1 - posiviteX,... , mip[n-1] - negativeZ, mip[n] - negativeZ
 */
struct texture_desc_t
{
	voidptr										data;
	s32											width, height;
	texture_format_e							format;
	filtering_e									min_filter, mag_filter;
	wrap_e										wrap_s, wrap_t, wrap_r;
	texture_dimension_e							dimension;
	texture_compression_e						compression;
	bool										has_mipmap;
};

// -------------------------------------------------------------------

struct framebuffer_desc_t
{
	color_attachment_list_t*					color_attachments;
	floral::vec4f								clear_color;
	s32											width, height;
	f32											scale;
	bool										has_depth;
	bool										color_has_mipmap = false;
};

// -------------------------------------------------------------------

struct shader_reflection_t
{
	shader_param_list_t*						textures;
	shader_param_list_t*						uniform_blocks;
};

// -------------------------------------------------------------------

struct shader_desc_t
{
	floral::path								vs_path, fs_path;
	cstr										vs, fs;

	shader_reflection_t							reflection;
};

/* material data WILL NOT be handled by insigne as this data is basically an instance of shader,
* they should be handled in user space
*/
template <typename t_value>
struct name_value_pair_t
{
	floral::crc_string							name;
	t_value										value;
};

template <typename t_value, u32 t_capacity>
using params_array_t = floral::inplace_array<name_value_pair_t<t_value>, t_capacity>;

struct ubmat_desc_t
{
	size										offset;
	size										range;
	ub_handle_t									ub_handle;
};

struct material_desc_t
{
	shader_handle_t								shader_handle;

	render_state_t								render_state;
	params_array_t<texture_handle_t, MAX_TEXTURE_SAMPLERS>	textures;
	params_array_t<ubmat_desc_t, MAX_UNIFORM_BLOCKS>		uniform_blocks;
};

//-------------------------------------------------------------------
struct vbdesc_t
{
	size										region_size;
	size										stride;
	voidptr										data;
	u32											count;
	buffer_usage_e								usage;
};

struct ibdesc_t
{
	size										region_size;
	voidptr										data;
	u32											count;
	buffer_usage_e								usage;
};

struct ubdesc_t
{
	size										region_size;
	voidptr										data;
	size										data_size;
	size										alignment;
	buffer_usage_e								usage;
};

//----------------------------------------------

struct framebuffer_descriptor_t {
	s32										width, height;
	f32										scale;
	bool									has_depth;
	color_attachment_list_t*				color_attachments;
};

struct surface_descriptor_t {
	size									vertex_stride;
	s32										vertices_count;
	s32										indices_count; // indices is always 'unsigned int'
};

struct texture2d_descriptor_t {
	s32										width, height;
	texture_format_e						format;
	filtering_e								min_filter, mag_filter;
	bool									has_mipmap;
};

struct texturecube_descriptor_t {
	s32										face_width, face_height;
	texture_format_e						format;
	filtering_e								min_filter, mag_filter;
	bool									has_mipmap;
};

// ------------------------------------------------------------------
}
