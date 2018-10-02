#pragma once

#include <floral.h>

namespace insigne {

// normal upload
const texture_handle_t						create_texture2d(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM = false); // deprecated
const texture_handle_t						create_texture2d(const texture2d_descriptor_t& i_desc, voidptr& o_placeholderData);
const texture_handle_t						upload_texture2d(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		voidptr i_data, const bool i_hasMM = false); // deprecated
const texture_handle_t						upload_texture2d(const texture2d_descriptor_t& i_desc, voidptr i_data);

const texture_handle_t						create_texturecube(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM = false);
const texture_handle_t						upload_texturecube(const s32 i_width, const s32 i_height,
		const texture_format_e i_format,
		const filtering_e i_minFilter, const filtering_e i_magFilter,
		voidptr i_data, const bool i_hasMM = false);

// streaming
voidptr										create_stream_texture2d(texture_format_e i_format);
void										update_stream_texture2d();

}
