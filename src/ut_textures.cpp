#include "insigne/ut_textures.h"

namespace insigne {

const texture_handle_t create_texture2d(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM /* = false */)
{
	voidptr placeholderData = s_composing_allocator.allocate(i_dataSize);
	texture_handle_t texHdl = upload_texture2d(i_width, i_height,
			i_format,
			i_minFilter, i_magFilter,
			placeholderData, i_hasMM);
	o_placeholderData = placeholderData;
	return texHdl;
}

const texture_handle_t upload_texture2d(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		voidptr i_data, const bool i_hasMM /* = false */)
{
	static texture_internal_format_e s_internal_formats[] = {
		texture_internal_format_e::rgb8,
		texture_internal_format_e::rgb16f,
		texture_internal_format_e::srgb8,
		texture_internal_format_e::rgba8,
		texture_internal_format_e::rgba16f,
		texture_internal_format_e::depth24,
		texture_internal_format_e::depth24_stencil8
	};
	static data_type_e s_data_types[] = {
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_signed_float,
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_signed_float,
		data_type_e::elem_unsigned_int,
		data_type_e::elem_unsigned_int_24_8
	};

	load_command cmd;
	cmd.data_type = stream_type::texture2d;
	// FIXME: what if user delete the i_data right after this call?
	cmd.data = i_data;
	cmd.format = i_format;
	cmd.width = i_width;
	cmd.height = i_height;
	cmd.internal_format = s_internal_formats[static_cast<s32>(i_format)];
	cmd.pixel_data_type = s_data_types[static_cast<s32>(i_format)];
	cmd.min_filter = i_minFilter;
	cmd.mag_filter = i_magFilter;
	cmd.has_builtin_mipmaps = i_hasMM;
	cmd.texture_idx = renderer::create_texture();
	push_command(cmd);

	return cmd.texture_idx;
}

const texture_handle_t create_texturecube(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM /* = false */)
{
	voidptr placeholderData = s_composing_allocator.allocate(i_dataSize * 6);
	texture_handle_t texHdl = upload_texturecube(i_width, i_height,
			i_format,
			i_minFilter, i_magFilter,
			placeholderData, i_hasMM);
	o_placeholderData = placeholderData;
	return texHdl;
}

const texture_handle_t upload_texturecube(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		voidptr i_data, const bool i_hasMM /* = false */)
{
	static texture_internal_format_e s_internal_formats[] = {
		texture_internal_format_e::rgb8,
		texture_internal_format_e::rgb16f,
		texture_internal_format_e::srgb8,
		texture_internal_format_e::rgba8,
		texture_internal_format_e::rgba16f,
		texture_internal_format_e::depth24,
		texture_internal_format_e::depth24_stencil8
	};
	static data_type_e s_data_types[] = {
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_signed_float,
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_unsigned_byte,
		data_type_e::elem_signed_float,
		data_type_e::elem_unsigned_int,
		data_type_e::elem_unsigned_int_24_8
	};

	load_command cmd;
	cmd.data_type = stream_type::texture_cube;
	// FIXME: what if user delete the i_data right after this call?
	cmd.data = i_data;
	cmd.format = i_format;
	cmd.width = i_width;
	cmd.height = i_height;
	cmd.internal_format = s_internal_formats[static_cast<s32>(i_format)];
	cmd.pixel_data_type = s_data_types[static_cast<s32>(i_format)];
	cmd.min_filter = i_minFilter;
	cmd.mag_filter = i_magFilter;
	cmd.has_builtin_mipmaps = i_hasMM;
	cmd.texture_idx = renderer::create_texture();
	push_command(cmd);

	return cmd.texture_idx;
}

}
