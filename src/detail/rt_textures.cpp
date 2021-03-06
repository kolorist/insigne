#include "insigne/detail/rt_textures.h"

#include <clover.h>

#include "insigne/memory.h"
#include "insigne/counters.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/internal_states.h"

namespace insigne
{
namespace detail
{
// -------------------------------------------------------------------

textures_pool_t									g_textures_pool;

static const GLenum s_GLFormat[] = {
	GL_RED,									// a
	GL_RG,									// rg
	GL_RG,									// hdr_rg
	GL_RGB,									// rgb
	GL_RGB,									// hdr_rgb
	GL_RGB,									// hdr_rgb_high
	GL_RGBA,								// hdr_rgba_high
	GL_RGB,									// hdr_rgb_half
	GL_RGB,									// srgb
	GL_RGBA,								// srgba
	GL_RGBA,								// rgba
	GL_RGBA,								// hdr_rgba
	GL_DEPTH_COMPONENT,						// depth
	GL_DEPTH_STENCIL						// depth_stencil
};

static const GLenum s_GLInternalFormat[] = {
	GL_R8,									// r
	GL_RG8,									// rg
	GL_RG16F,								// hdr_rg
	GL_RGB8,								// rgb
	GL_RGB16F,								// hdr_rgb
	GL_RGB16F,								// hdr_rgb_high
	GL_RGBA16F,								// hdr_rgba_high
	GL_R11F_G11F_B10F,						// hdr_rgb_half
	GL_SRGB8,								// srgb
	GL_SRGB8_ALPHA8,						// srgba
	GL_RGBA8,								// rgba
	GL_RGBA16F,								// hdr_rgba
	GL_DEPTH_COMPONENT24,					// depth
	GL_DEPTH24_STENCIL8						// depth_stencil
};

static const GLenum s_GLDataType[] = {
	GL_UNSIGNED_BYTE,						// r
	GL_UNSIGNED_BYTE,						// rg
	GL_HALF_FLOAT,							// hdr_rg
	GL_UNSIGNED_BYTE,						// rgb
	GL_HALF_FLOAT,							// hdr_rgb
	GL_FLOAT,								// hdr_rgb_high
	GL_FLOAT,								// hdr_rgba_high
	GL_HALF_FLOAT,							// hdr_rgb_half
	GL_UNSIGNED_BYTE,						// srgb
	GL_UNSIGNED_BYTE,						// srgba
	GL_UNSIGNED_BYTE,						// rgba
	GL_HALF_FLOAT,							// hdr_rgba
	GL_UNSIGNED_INT,						// depth
	GL_UNSIGNED_INT_24_8					// depth_stencil
};

// data size in the CPU, not GPU!!!
static const size s_DataSize[] = {
	1,										// r
	2,										// rg
	4,										// hdr_rg
	3,										// rgb
	6,										// hdr_rgb
	12,										// hdr_rgb_high
	16,										// hdr_rgba_high
	6,										// hdr_rgb_half
	3,										// srgb
	4,										// srgba
	4,										// rgba
	8,										// hdr_rgba
	4,										// depth
	4										// depth_stencil
};

static const GLenum s_GLFiltering[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

static const GLenum s_GLWrap[] = {
	GL_CLAMP_TO_EDGE,
	GL_MIRRORED_REPEAT,
	GL_REPEAT
};

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_textures_command_buffer(const size i_cmdBuffId) {
	return detail::g_textures_command_buffer[i_cmdBuffId];
}

// ---------------------------------------------
/* ut */
const texture_handle_t create_texture()
{
	ssize idx = g_textures_pool.get_size();
	g_textures_pool.push_back(texture_desc_t());

	return texture_handle_t(idx);
}

/* ut */
const texture_handle_t get_last_texture()
{
	return g_textures_pool.get_size() - 1;
}

// ---------------------------------------------
void upload_texture(const texture_handle_t i_hdl, const insigne::texture_desc_t& i_uploadDesc)
{
	texture_desc_t& texDesc = g_textures_pool[(s32)i_hdl];

	texDesc.width = i_uploadDesc.width;
	texDesc.height = i_uploadDesc.height;
	texDesc.depth = i_uploadDesc.depth;
	texDesc.format = i_uploadDesc.format;
	texDesc.min_filter = i_uploadDesc.min_filter;
	texDesc.mag_filter = i_uploadDesc.mag_filter;
	texDesc.dimension = i_uploadDesc.dimension;
	texDesc.compression = i_uploadDesc.compression;
	texDesc.has_mipmap = i_uploadDesc.has_mipmap;
	texDesc.wrap_s = i_uploadDesc.wrap_s;
	texDesc.wrap_t = i_uploadDesc.wrap_t;
	texDesc.wrap_r = i_uploadDesc.wrap_r;

	GLenum compressFormat = GL_RGBA;
	size blockSize = 0;
	bool hasCompression = true;
	switch (i_uploadDesc.compression)
	{
	case texture_compression_e::dxt:
	{
		switch (i_uploadDesc.format)
		{
		case texture_format_e::rgb:
		{
			compressFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			blockSize = 8;
			break;
		}
		case texture_format_e::rgba:
		{
			compressFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			blockSize = 16;
			break;
		}
		default:
			FLORAL_ASSERT(false);
			break;
		}
		break;
	}

	case texture_compression_e::etc:
	{
		switch (i_uploadDesc.format)
		{
		case texture_format_e::rgb:
		{
			compressFormat = GL_COMPRESSED_RGB8_ETC2;
			blockSize = 8;
			break;
		}
		case texture_format_e::rgba:
		{
			compressFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			blockSize = 16;
			break;
		}
		default:
			FLORAL_ASSERT(false);
			break;
		}
		break;
	}

	case texture_compression_e::no_compression:
	default:
		hasCompression = false;
		break;
	}

	GLuint newTexture = 0;
	pxGenTextures(1, &newTexture);

	// TODO: dxt and etc2 compressed image size is same with each other???

	if (i_uploadDesc.dimension == texture_dimension_e::tex_2d)
	{
		pxBindTexture(GL_TEXTURE_2D, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_GLFiltering[s32(i_uploadDesc.mag_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_GLFiltering[s32(i_uploadDesc.min_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s_GLWrap[s32(i_uploadDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, s_GLWrap[s32(i_uploadDesc.wrap_t)]);

		if (i_uploadDesc.has_mipmap)
		{
			s32 texSize = i_uploadDesc.width;
			s32 mipIdx = 0;
			size offset = 0;
			while (texSize >= 1)
			{
				// NOTE: please remember that: when loading mipmaps, the width and height is
				// resolution of the mipmap, not the resolution of the largest mip
				if (!hasCompression)
				{
					if (i_uploadDesc.data)
					{
						pxTexImage2D(GL_TEXTURE_2D, mipIdx, s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
								(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
					}
					else
					{
						pxTexImage2D(GL_TEXTURE_2D, mipIdx, s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
								nullptr);
					}
					offset += texSize * texSize * s_DataSize[(s32)i_uploadDesc.format];
				}
				else
				{
					size compressedDataSize = ((texSize + 3) / 4) * ((texSize + 3) / 4) * blockSize;
					if (i_uploadDesc.data)
					{
						pxCompressedTexImage2D(GL_TEXTURE_2D, mipIdx, compressFormat,
								texSize, texSize, 0,
								compressedDataSize, (void*)((aptr)i_uploadDesc.data + (aptr)offset));
					}
					else
					{
						pxCompressedTexImage2D(GL_TEXTURE_2D, mipIdx, compressFormat,
								texSize, texSize, 0,
								0, nullptr);
					}
					offset += compressedDataSize;
				}
				texSize >>= 1;
				mipIdx++;
			}
		}
		else
		{
			if (!hasCompression)
			{
				if (i_uploadDesc.data)
				{
					pxTexImage2D(GL_TEXTURE_2D, 0, s_GLInternalFormat[s32(i_uploadDesc.format)],
							i_uploadDesc.width, i_uploadDesc.height, 0,
							s_GLFormat[s32(i_uploadDesc.format)], s_GLDataType[s32(i_uploadDesc.format)],
							(GLvoid*)i_uploadDesc.data);
				}
				else
				{
					pxTexImage2D(GL_TEXTURE_2D, 0, s_GLInternalFormat[s32(i_uploadDesc.format)],
							i_uploadDesc.width, i_uploadDesc.height, 0,
							s_GLFormat[s32(i_uploadDesc.format)], s_GLDataType[s32(i_uploadDesc.format)],
							nullptr);
				}
			}
			else
			{
				size compressedDataSize = ((i_uploadDesc.width + 3) / 4) * ((i_uploadDesc.height + 3) / 4) * blockSize;
				if (i_uploadDesc.data)
				{
					pxCompressedTexImage2D(GL_TEXTURE_2D, 0, compressFormat,
							i_uploadDesc.width, i_uploadDesc.height, 0,
							compressedDataSize, (void*)(i_uploadDesc.data));
				}
				else
				{
					pxCompressedTexImage2D(GL_TEXTURE_2D, 0, compressFormat,
							i_uploadDesc.width, i_uploadDesc.height, 0,
							0, nullptr);
				}
			}
		}
		pxBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (i_uploadDesc.dimension == texture_dimension_e::tex_cube)
	{
		pxBindTexture(GL_TEXTURE_CUBE_MAP, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, s_GLFiltering[(s32)i_uploadDesc.mag_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, s_GLFiltering[(s32)i_uploadDesc.min_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, s_GLWrap[s32(i_uploadDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, s_GLWrap[s32(i_uploadDesc.wrap_t)]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, s_GLWrap[s32(i_uploadDesc.wrap_r)]);
		if (i_uploadDesc.has_mipmap)
		{
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++)
			{
				s32 texSize = i_uploadDesc.width;
				s32 mipIdx = 0;
				while (texSize >= 1)
				{
					if (!hasCompression)
					{
						if (i_uploadDesc.data)
						{
							pxTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, mipIdx,
									s_GLInternalFormat[(s32)i_uploadDesc.format],
									texSize, texSize, 0,
									s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
									(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
						}
						else
						{
							pxTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, mipIdx,
									s_GLInternalFormat[(s32)i_uploadDesc.format],
									texSize, texSize, 0,
									s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
									nullptr);
						}
						offset += texSize * texSize * s_DataSize[(s32)i_uploadDesc.format];
					}
					else
					{
						size compressedDataSize = ((texSize + 3) / 4) * ((texSize + 3) / 4) * blockSize;
						if (i_uploadDesc.data)
						{
							pxCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, mipIdx, compressFormat,
									texSize, texSize, 0,
									compressedDataSize, (void*)((aptr)i_uploadDesc.data + (aptr)offset));
						}
						else
						{
							pxCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, mipIdx, compressFormat,
									texSize, texSize, 0,
									0, nullptr);
						}
						offset += compressedDataSize;
					}
					texSize >>= 1;
					mipIdx++;
				}
			}
		}
		else
		{
			s32 texSize = i_uploadDesc.width;
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++)
			{
				if (!hasCompression)
				{
					if (i_uploadDesc.data)
					{
						pxTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0,
								s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
								(GLvoid*)((aptr)i_uploadDesc.data + (aptr)offset));
					}
					else
					{
						pxTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0,
								s_GLInternalFormat[(s32)i_uploadDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)i_uploadDesc.format], s_GLDataType[(s32)i_uploadDesc.format],
								nullptr);
					}
					offset += texSize * texSize * s_DataSize[(s32)i_uploadDesc.format];
				}
				else
				{
					size compressedDataSize = ((texSize + 3) / 4) * ((texSize + 3) / 4) * blockSize;
					if (i_uploadDesc.data)
					{
						pxCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0, compressFormat,
								texSize, texSize, 0,
								compressedDataSize, (void*)(i_uploadDesc.data));
					}
					else
					{
						pxCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0, compressFormat,
								texSize, texSize, 0,
								0, nullptr);
					}
					offset += compressedDataSize;
				}
			}
		}
		pxBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else if (i_uploadDesc.dimension == texture_dimension_e::tex_3d)
	{
		pxBindTexture(GL_TEXTURE_3D, newTexture);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, s_GLFiltering[s32(i_uploadDesc.mag_filter)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, s_GLFiltering[s32(i_uploadDesc.min_filter)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, s_GLWrap[s32(i_uploadDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, s_GLWrap[s32(i_uploadDesc.wrap_t)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, s_GLWrap[s32(i_uploadDesc.wrap_r)]);

		FLORAL_ASSERT(i_uploadDesc.has_mipmap == false); // we won't support mipmapping for 3d texture
		FLORAL_ASSERT(hasCompression == false); // we won't support compression for 3d texture

		if (i_uploadDesc.data)
		{
			pxTexImage3D(GL_TEXTURE_3D, 0, s_GLInternalFormat[s32(i_uploadDesc.format)],
					i_uploadDesc.width, i_uploadDesc.height, i_uploadDesc.depth, 0,
					s_GLFormat[s32(i_uploadDesc.format)], s_GLDataType[s32(i_uploadDesc.format)],
					(GLvoid*)i_uploadDesc.data);
		}
		else
		{
			pxTexImage3D(GL_TEXTURE_3D, 0, s_GLInternalFormat[s32(i_uploadDesc.format)],
					i_uploadDesc.width, i_uploadDesc.height, i_uploadDesc.depth, 0,
					s_GLFormat[s32(i_uploadDesc.format)], s_GLDataType[s32(i_uploadDesc.format)],
					nullptr);
		}
	}
	else
	{
		FLORAL_ASSERT(false);
		// nani?!
	}

	texDesc.gpu_handle = newTexture;
}

void update_texture(const texture_handle_t i_hdl, const voidptr i_data, const size i_dataSize)
{
	texture_desc_t& texDesc = g_textures_pool[(s32)i_hdl];

	//TODO: assert i_dataSize
	GLenum compressFormat = GL_RGBA;
	size blockSize = 0;
	bool hasCompression = true;
	switch (texDesc.compression)
	{
	case texture_compression_e::dxt:
	{
		switch (texDesc.format)
		{
		case texture_format_e::rgb:
		{
			compressFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			blockSize = 8;
			break;
		}
		case texture_format_e::rgba:
		{
			compressFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			blockSize = 16;
			break;
		}
		default:
			FLORAL_ASSERT(false);
			break;
		}
		break;
	}

	case texture_compression_e::etc:
	{
		switch (texDesc.format)
		{
		case texture_format_e::rgb:
		{
			compressFormat = GL_COMPRESSED_RGB8_ETC2;
			blockSize = 8;
			break;
		}
		case texture_format_e::rgba:
		{
			compressFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			blockSize = 16;
			break;
		}
		default:
			FLORAL_ASSERT(false);
			break;
		}
		break;
	}

	case texture_compression_e::no_compression:
	default:
		hasCompression = false;
		break;
	}

	if (texDesc.dimension == texture_dimension_e::tex_2d)
	{
		pxBindTexture(GL_TEXTURE_2D, texDesc.gpu_handle);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_GLFiltering[s32(texDesc.mag_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_GLFiltering[s32(texDesc.min_filter)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s_GLWrap[s32(texDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, s_GLWrap[s32(texDesc.wrap_t)]);

		if (texDesc.has_mipmap)
		{
			s32 texSize = texDesc.width;
			s32 mipIdx = 0;
			size offset = 0;
			while (texSize >= 1)
			{
				// NOTE: please remember that: when loading mipmaps, the width and height is
				// resolution of the mipmap, not the resolution of the largest mip
				if (!hasCompression)
				{
					if (i_data)
					{
						pxTexImage2D(GL_TEXTURE_2D, mipIdx, s_GLInternalFormat[(s32)texDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)texDesc.format], s_GLDataType[(s32)texDesc.format],
								(GLvoid*)((aptr)i_data + (aptr)offset));
					}
					else
					{
						pxTexImage2D(GL_TEXTURE_2D, mipIdx, s_GLInternalFormat[(s32)texDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)texDesc.format], s_GLDataType[(s32)texDesc.format],
								nullptr);
					}
					offset += texSize * texSize * s_DataSize[(s32)texDesc.format];
				}
				else
				{
					size compressedDataSize = ((texSize + 3) / 4) * ((texSize + 3) / 4) * blockSize;
					if (i_data)
					{
						pxCompressedTexImage2D(GL_TEXTURE_2D, mipIdx, compressFormat,
								texSize, texSize, 0,
								compressedDataSize, (GLvoid*)((aptr)i_data + (aptr)offset));
					}
					else
					{
						pxCompressedTexImage2D(GL_TEXTURE_2D, mipIdx, compressFormat,
								texSize, texSize, 0,
								0, nullptr);
					}
					offset += compressedDataSize;
				}
				texSize >>= 1;
				mipIdx++;
			}
		}
		else
		{
			if (!hasCompression)
			{
				if (i_data)
				{
					pxTexImage2D(GL_TEXTURE_2D, 0, s_GLInternalFormat[s32(texDesc.format)],
							texDesc.width, texDesc.height, 0,
							s_GLFormat[s32(texDesc.format)], s_GLDataType[s32(texDesc.format)],
							(GLvoid*)i_data);
				}
				else
				{
					pxTexImage2D(GL_TEXTURE_2D, 0, s_GLInternalFormat[s32(texDesc.format)],
							texDesc.width, texDesc.height, 0,
							s_GLFormat[s32(texDesc.format)], s_GLDataType[s32(texDesc.format)],
							nullptr);
				}
			}
			else
			{
				size compressedDataSize = ((texDesc.width + 3) / 4) * ((texDesc.height + 3) / 4) * blockSize;
				if (i_data)
				{
					pxCompressedTexImage2D(GL_TEXTURE_2D, 0, compressFormat,
							texDesc.width, texDesc.height, 0,
							compressedDataSize, (void*)(i_data));
				}
				else
				{
					pxCompressedTexImage2D(GL_TEXTURE_2D, 0, compressFormat,
							texDesc.width, texDesc.height, 0,
							0, nullptr);
				}
			}
		}

		pxBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (texDesc.dimension == texture_dimension_e::tex_cube)
	{
		pxBindTexture(GL_TEXTURE_CUBE_MAP, texDesc.gpu_handle);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, s_GLFiltering[(s32)texDesc.mag_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, s_GLFiltering[(s32)texDesc.min_filter]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, s_GLWrap[s32(texDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, s_GLWrap[s32(texDesc.wrap_t)]);
		pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, s_GLWrap[s32(texDesc.wrap_r)]);
		if (texDesc.has_mipmap) {
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
				s32 texSize = texDesc.width;
				s32 mipIdx = 0;
				while (texSize >= 1) {
					if (i_data) {
						pxTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
								mipIdx,
								s_GLInternalFormat[(s32)texDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)texDesc.format],
								s_GLDataType[(s32)texDesc.format],
								(GLvoid*)((aptr)i_data + (aptr)offset));
					} else {
						pxTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
								mipIdx,
								s_GLInternalFormat[(s32)texDesc.format],
								texSize, texSize, 0,
								s_GLFormat[(s32)texDesc.format],
								s_GLDataType[(s32)texDesc.format],
								nullptr);
					}
					offset += texSize * texSize * s_DataSize[(s32)texDesc.format];
					texSize >>= 1;
					mipIdx++;
				}
			}
		} else {
			s32 texSize = texDesc.width;
			size offset = 0;
			for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
				if (i_data) {
					pxTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
							0,
							s_GLInternalFormat[(s32)texDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)texDesc.format],
							s_GLDataType[(s32)texDesc.format],
							(GLvoid*)((aptr)i_data + (aptr)offset));
				} else {
					pxTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
							0,
							s_GLInternalFormat[(s32)texDesc.format],
							texSize, texSize, 0,
							s_GLFormat[(s32)texDesc.format],
							s_GLDataType[(s32)texDesc.format],
							nullptr);
				}
				offset += texSize * texSize * s_DataSize[(s32)texDesc.format];
			}
		}
		pxBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else if (texDesc.dimension == texture_dimension_e::tex_3d)
	{
		pxBindTexture(GL_TEXTURE_3D, texDesc.gpu_handle);

		// unpacking settings
		pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, s_GLFiltering[s32(texDesc.mag_filter)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, s_GLFiltering[s32(texDesc.min_filter)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, s_GLWrap[s32(texDesc.wrap_s)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, s_GLWrap[s32(texDesc.wrap_t)]);
		pxTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, s_GLWrap[s32(texDesc.wrap_r)]);

		FLORAL_ASSERT(texDesc.has_mipmap == false); // we won't support mipmapping for 3d texture
		FLORAL_ASSERT(hasCompression == false); // we won't support compression for 3d texture

		if (i_data)
		{
			pxTexImage3D(GL_TEXTURE_3D, 0, s_GLInternalFormat[s32(texDesc.format)],
					texDesc.width, texDesc.height, texDesc.depth, 0,
					s_GLFormat[s32(texDesc.format)], s_GLDataType[s32(texDesc.format)],
					(GLvoid*)i_data);
		}
		else
		{
			pxTexImage3D(GL_TEXTURE_3D, 0, s_GLInternalFormat[s32(texDesc.format)],
					texDesc.width, texDesc.height, texDesc.depth, 0,
					s_GLFormat[s32(texDesc.format)], s_GLDataType[s32(texDesc.format)],
					nullptr);
		}
	}
	else
	{
		// nani?!
	}
}

static void clean_up_snapshot(const texture_handle_t i_textureHandle)
{
	CLOVER_VERBOSE("Cleaning up textures snapshot...");
	while (i_textureHandle != g_textures_pool.get_size() - 1)
	{
		texture_desc_t texDesc = g_textures_pool.pop_back();
		CLOVER_VERBOSE("Deleting texture id %d", texDesc.gpu_handle);
		pxDeleteTextures(1, &texDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Finished cleaning up textures snapshot");
}

// ---------------------------------------------
void initialize_textures_module()
{
	// TODO: hardcode!!!
	//g_textures_pool.init(64u, &g_persistance_allocator);
}

void cleanup_textures_module()
{
	CLOVER_VERBOSE("Cleaning up textures module...");
	for (ssize i = 0; i < g_textures_pool.get_size(); i++)
	{
		texture_desc_t& texDesc = g_textures_pool[i];
		CLOVER_VERBOSE("Deleting texture id %d", texDesc.gpu_handle);
		pxDeleteTextures(1, &texDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd textures", g_textures_pool.get_size());
	CLOVER_VERBOSE("Finished cleaning up textures module...");
}

void process_textures_command_buffer(const size i_cmdBuffId)
{
	detail::gpu_command_buffer_t& cmdbuff = get_textures_command_buffer(i_cmdBuffId);

	u64 writeSlot = g_global_counters.current_write_slot;
	g_debug_frame_counters[writeSlot].num_texture_commands += (u32)cmdbuff.get_size();

	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::textures_command:
			{
				textures_command_t cmd;
				gpuCmd.serialize(cmd);

				switch (cmd.command_type) {
					case textures_command_type_e::create_texture:
					{
						upload_texture(cmd.create_texture_data.texture_handle, cmd.create_texture_data.desc);
						break;
					}
					case textures_command_type_e::stream_texture:
					{
						update_texture(cmd.stream_texture_data.texture_handle, cmd.stream_texture_data.data, cmd.stream_texture_data.dataSize);
						break;
					}
					case textures_command_type_e::clean_up_snapshot:
					{
						clean_up_snapshot(cmd.clean_up_snapshot_data.downto_handle);
						break;
					}
					default:
						break;
				}
			}
			default:
				break;
		}
	}

	cmdbuff.clear();
}

}
}
