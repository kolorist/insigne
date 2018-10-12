#include "insigne/detail/rt_textures.h"

#include <clover.h>

#include "insigne/memory.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/internal_states.h"

namespace insigne {
namespace detail {

textures_pool_t									g_textures_pool;

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_textures_command_buffer(const size i_cmdBuffId) {
	return detail::g_textures_command_buffer[i_cmdBuffId];
}

// ---------------------------------------------
const texture_handle_t create_texture()
{
	u32 idx = g_textures_pool.get_size();
	g_textures_pool.push_back(texture_desc_t());

	return texture_handle_t(idx);
}

// ---------------------------------------------
void upload_texture(const texture_handle_t i_hdl, const insigne::texture_desc_t& i_uploadDesc)
{
	texture_desc_t& texDesc = g_textures_pool[(s32)i_hdl];
	static GLenum s_GLFormat[] = {
		GL_RG,									// rg
		GL_RG,									// hdr_rg
		GL_RGB,									// rgb
		GL_RGB,									// hdr_rgb
		GL_RGB,									// srgb
		GL_RGBA,								// rgba
		GL_RGBA,								// hdr_rgba
		GL_DEPTH_COMPONENT,						// depth
		GL_DEPTH_STENCIL						// depth_stencil
	};

	static GLenum s_GLInternalFormat[] = {
		GL_RG8,									// rg
		GL_RG16F,								// hdr_rg
		GL_RGB8,								// rgb
		GL_RGB16F,								// hdr_rgb
		GL_SRGB8,								// srgb
		GL_RGBA8,								// rgba
		GL_RGBA16F,								// hdr_rgba
		GL_DEPTH_COMPONENT24,					// depth
		GL_DEPTH24_STENCIL8						// depth_stencil
	};

	static GLenum s_GLDataType[] = {
		GL_UNSIGNED_BYTE,						// rg
		GL_FLOAT,								// hdr_rg
		GL_UNSIGNED_BYTE,						// rgb
		GL_FLOAT,								// hdr_rgb
		GL_UNSIGNED_BYTE,						// srgb
		GL_UNSIGNED_BYTE,						// rgba
		GL_FLOAT,								// hdr_rgba
		GL_UNSIGNED_INT,						// depth
		GL_UNSIGNED_INT_24_8					// depth_stencil
	};

	static size s_NumChannels[] = {
		2,										// rg
		2,										// hdr_rg
		3,										// rgb
		3,										// hdr_rgb
		3,										// srgb
		4,										// rgba
		4,										// hdr_rgba
		1,										// depth
		1										// depth_stencil
	};

	static size s_DataSize[] = {
		1,
		4,
		1,
		4,
		1,
		1,
		4,
		4,
		4
	};

	static GLenum s_GLFiltering[] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	texDesc.width = i_uploadDesc.width;
	texDesc.height = i_uploadDesc.height;
	texDesc.format = i_uploadDesc.format;
	texDesc.min_filter = i_uploadDesc.min_filter;
	texDesc.mag_filter = i_uploadDesc.mag_filter;
	texDesc.dimension = i_uploadDesc.dimension;
	texDesc.has_mipmap = i_uploadDesc.has_mipmap;

	GLuint newTexture = 0;
	
	pxGenTextures(1, &newTexture);

	if (i_uploadDesc.dimension == texture_dimension_e::tex_2d) {
		pxBindTexture(GL_TEXTURE_2D, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_GLFiltering[s32(i_uploadDesc.mag_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_GLFiltering[s32(i_uploadDesc.min_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (i_uploadDesc.has_mipmap) {
			s32 texSize = i_uploadDesc.width;
			s32 mipIdx = 0;
			size offset = 0;
			while (texSize >= 1) {
				// NOTE: please remember that: when loading mipmaps, the width and height is
				// resolution of the mipmap, not the resolution of the largest mip
				if (i_uploadDesc.data) {
					pxTexImage2D(GL_TEXTURE_2D, mipIdx,
							s_GLInternalFormat[(s32)i_uploadDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)i_uploadDesc.format],
							s_GLDataType[(s32)i_uploadDesc.format],
							(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
				} else {
					pxTexImage2D(GL_TEXTURE_2D, mipIdx,
							s_GLInternalFormat[(s32)i_uploadDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)i_uploadDesc.format],
							s_GLDataType[(s32)i_uploadDesc.format],
							nullptr);
				}
				offset += texSize * texSize * s_NumChannels[(s32)i_uploadDesc.format] * s_DataSize[(s32)i_uploadDesc.format];
				texSize >>= 1;
				mipIdx++;
			}
		} else {
			if (i_uploadDesc.data) {
				pxTexImage2D(GL_TEXTURE_2D, 0,
						s_GLInternalFormat[s32(i_uploadDesc.format)],
						i_uploadDesc.width, i_uploadDesc.height,
						0,
						s_GLFormat[s32(i_uploadDesc.format)],
						s_GLDataType[s32(i_uploadDesc.format)],
						(GLvoid*)i_uploadDesc.data);
			} else {
				pxTexImage2D(GL_TEXTURE_2D, 0,
						s_GLInternalFormat[s32(i_uploadDesc.format)],
						i_uploadDesc.width, i_uploadDesc.height,
						0,
						s_GLFormat[s32(i_uploadDesc.format)],
						s_GLDataType[s32(i_uploadDesc.format)],
						nullptr);
			}
		}

		pxBindTexture(GL_TEXTURE_2D, 0);
	} else if (i_uploadDesc.dimension == texture_dimension_e::tex_cube) {
		pxBindTexture(GL_TEXTURE_CUBE_MAP, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, s_GLFiltering[(s32)i_uploadDesc.mag_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, s_GLFiltering[(s32)i_uploadDesc.min_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (i_uploadDesc.has_mipmap) {
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
				s32 texSize = i_uploadDesc.width;
				s32 mipIdx = 0;
				while (texSize >= 1) {
					if (i_uploadDesc.data) {
						pxTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
								mipIdx,
								s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format],
								s_GLDataType[(s32)i_uploadDesc.format],
								(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
					} else {
						pxTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
								mipIdx,
								s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format],
								s_GLDataType[(s32)i_uploadDesc.format],
								nullptr);
					}
					offset += texSize * texSize * s_NumChannels[(s32)i_uploadDesc.format] * s_DataSize[(s32)i_uploadDesc.format];
					texSize >>= 1;
					mipIdx++;
				}
			}
		} else {
			s32 texSize = i_uploadDesc.width;
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
				if (i_uploadDesc.data) {
					pxTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
							0,
							s_GLInternalFormat[(s32)i_uploadDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)i_uploadDesc.format],
							s_GLDataType[(s32)i_uploadDesc.format],
							(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
				} else {
					pxTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
							0,
							s_GLInternalFormat[(s32)i_uploadDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)i_uploadDesc.format],
							s_GLDataType[(s32)i_uploadDesc.format],
							nullptr);
				}
				offset += texSize * texSize * s_NumChannels[(s32)i_uploadDesc.format] * s_DataSize[(s32)i_uploadDesc.format];
			}
		}
		pxBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	} else if (i_uploadDesc.dimension == texture_dimension_e::tex_3d) {
		// TODO: texture_dimension_e::tex_3d?
	} else {
		// nani?!
	}

	texDesc.gpu_handle = newTexture;
}

// ---------------------------------------------
void initialize_textures_module()
{
	// TODO: hardcode!!!
	g_textures_pool.init(64u, &g_persistance_allocator);
}

void process_textures_command_buffer(const size i_cmdBuffId)
{
	detail::gpu_command_buffer_t& cmdbuff = get_textures_command_buffer(i_cmdBuffId);
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::textures_command:
				{
					textures_command_t cmd;
					gpuCmd.serialize(cmd);

					upload_texture(cmd.create_texture_data.texture_handle, cmd.create_texture_data.desc);
					break;
				}
			default:
				break;
		}
	}

	cmdbuff.empty();
}

}
}
