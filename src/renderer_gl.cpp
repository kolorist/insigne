#include "insigne/renderer.h"

#include <floral.h>
#include <clover.h>

#include "memory.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/detail/renderer_states_gl.h"

namespace insigne {
namespace renderer {

// -----------------------------------------
static GLenum s_draw_types[] = {
	GL_STATIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_STREAM_DRAW };

static GLenum s_cmp_funcs[] = {
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static GLenum s_front_faces[] = {
	GL_CW,
	GL_CCW
};

static GLenum s_face_sides[] = {
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};

static GLenum s_blend_equations[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

static GLenum s_factors[] = {
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA
};

static GLenum s_stencil_ops[] = {
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_INCR_WRAP,
	GL_DECR,
	GL_DECR_WRAP,
	GL_INVERT
};

static GLenum s_gl_texture_formats[] = {
	GL_RGB,
	GL_RGB,
	GL_RGB,
	GL_RGBA,
	GL_RGBA,
	GL_DEPTH_COMPONENT,
	GL_DEPTH_STENCIL
};

static GLenum s_gl_internal_formats[] = {
	GL_RG16F,
	GL_RGB16F,
	GL_RGBA16F,
	GL_RGB8,
	GL_RGBA8,
	GL_SRGB8,
	GL_SRGB8_ALPHA8,
	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
	GL_DEPTH24_STENCIL8
};

static GLenum s_gl_data_types[] = {
	GL_UNSIGNED_BYTE,
	GL_BYTE,
	GL_UNSIGNED_INT,
	GL_INT,
	GL_FLOAT,
	GL_UNSIGNED_INT_24_8
};

static GLenum s_filterings[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

// -----------------------------------------

void initialize_renderer()
{
	detail::s_shaders.init(64u, &g_persistance_allocator);
	detail::s_material_templates.init(64u, &g_persistance_allocator);
	detail::s_textures.init(64u, &g_persistance_allocator);
	detail::s_surfaces.init(256u, &g_persistance_allocator);
	detail::s_framebuffers.init(32u, &g_persistance_allocator);
}

// -----------------------------------------
template <>
void set_depth_test<true_type>(const compare_func_e i_depthFunc)
{
	pxEnable(GL_DEPTH_TEST);
	pxDepthFunc(s_cmp_funcs[static_cast<s32>(i_depthFunc)]);
}

template <>
void set_depth_test<false_type>(const compare_func_e i_depthFunc)
{
	pxDisable(GL_DEPTH_TEST);
}

template <>
void set_depth_write<true_type>()
{
	pxDepthMask(GL_TRUE);
}

template <>
void set_depth_write<false_type>()
{
	pxDepthMask(GL_FALSE);
}

template <>
void set_cull_face<true_type>(const face_side_e i_faceSide, const front_face_e i_frontFace)
{
	pxEnable(GL_CULL_FACE);
	pxCullFace(s_face_sides[(s32)i_faceSide]);
	pxFrontFace(s_front_faces[(s32)i_frontFace]);
}

template <>
void set_cull_face<false_type>(const face_side_e i_faceSide, const front_face_e i_frontFace)
{
	pxDisable(GL_CULL_FACE);
}

template <>
void set_blending<true_type>(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor)
{
	pxEnable(GL_BLEND);
	pxBlendEquation(s_blend_equations[static_cast<s32>(i_blendEqu)]);
	pxBlendFunc(s_factors[static_cast<s32>(i_sfactor)], s_factors[static_cast<s32>(i_dfactor)]);
}

template <>
void set_blending<false_type>(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor)
{
	pxDisable(GL_BLEND);
}

template <>
void set_scissor_test<true_type>(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	pxEnable(GL_SCISSOR_TEST);
	pxScissor(i_x, i_y, i_width, i_height);
}

template <>
void set_scissor_test<false_type>(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	pxDisable(GL_SCISSOR_TEST);
}

template <>
void set_stencil_test<true_type>(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass)
{
	pxEnable(GL_STENCIL_TEST);
	pxStencilFunc(s_cmp_funcs[static_cast<s32>(i_func)], i_ref, i_mask);
	pxStencilOp(s_stencil_ops[static_cast<s32>(i_sfail)], s_stencil_ops[static_cast<s32>(i_dpfail)], s_stencil_ops[static_cast<s32>(i_dppass)]);
}

template <>
void set_stencil_test<false_type>(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass)
{
	pxDisable(GL_STENCIL_TEST);
}

void enable_vertex_attrib(const u32 i_location)
{
	pxEnableVertexAttribArray(i_location);
}

void describe_vertex_data(const u32 i_location, const s32 i_size,
		const data_type_e i_type, const bool i_normalized, const s32 i_stride, const voidptr offset)
{
	pxVertexAttribPointer(i_location, i_size, s_gl_data_types[static_cast<s32>(i_type)],
			i_normalized? GL_TRUE : GL_FALSE,
			i_stride, (const GLvoid*)offset);
}
// -----------------------------------------

void clear_color(const floral::vec4f& i_color)
{
	pxClearColor(i_color.x, i_color.y, i_color.z, i_color.w);
}

void clear_framebuffer(const bool i_clearcolor, const bool i_cleardepth)
{
	GLbitfield clearBit = 0;
	if (i_clearcolor) clearBit |= GL_COLOR_BUFFER_BIT;
	if (i_cleardepth) {
		set_depth_write<true_type>();
		clearBit |= GL_DEPTH_BUFFER_BIT;
	}
	pxClearDepthf(1.0f);
	pxClear(clearBit);
}

const framebuffer_handle_t create_framebuffer(const s32 i_colorAttachsCount)
{
	u32 idx = detail::s_framebuffers.get_size();
	detail::framebuffer newFramebuffer;
	for (s32 i = 0; i < i_colorAttachsCount; i++) {
		newFramebuffer.color_attachments[i] = create_texture();
	}
	newFramebuffer.depthstencil_attachment = create_texture();
	newFramebuffer.color_attachments_count = i_colorAttachsCount;
	detail::s_framebuffers.push_back(newFramebuffer);
	return (framebuffer_handle_t)idx;
}

void init_framebuffer(const framebuffer_handle_t i_hdl, const s32 i_width, const s32 i_height, const f32 i_scale,
		const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs)
{
	detail::framebuffer& thisFramebuffer = detail::s_framebuffers[i_hdl];
	// at this point, we already have color attachments count
	u32 colorAttachsCount = 0;
	if (i_colorAttachs)
		colorAttachsCount = i_colorAttachs->get_size();
	// TODO: assertion to check colorAttachsCount and thisFramebuffer.color_attachments_count

	GLuint newFBO = 0;
	pxGenFramebuffers(1, &newFBO);
	pxBindFramebuffer(GL_FRAMEBUFFER, newFBO);

	s32 swidth = (s32)((f32)i_width * i_scale);
	s32 sheight = (s32)((f32)i_height * i_scale);

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

	for (u32 i = 0; i < colorAttachsCount; i++) {
		// TODO: use upload_texture2d

		texture_format_e texFormat = i_colorAttachs->at(i).texture_format;
		texture_internal_format_e internalFormat = s_internal_formats[(s32)texFormat];
		data_type_e dataType = s_data_types[(s32)texFormat];
		texture_handle_t thisTexture = thisFramebuffer.color_attachments[i];
		upload_texture2d(thisTexture,
				swidth, sheight, texFormat, internalFormat, dataType, nullptr,
				filtering_e::linear, filtering_e::linear);
		// attach texture to fbo
		detail::texture& colorTex = detail::s_textures[thisTexture];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTex.gpu_handle, 0);
	}

	// TODO: depth texture
	{
		texture_format_e texFormat = texture_format_e::depth;
		texture_internal_format_e internalFormat = s_internal_formats[(s32)texFormat];
		data_type_e dataType = s_data_types[(s32)texFormat];
		texture_handle_t depthTexture = thisFramebuffer.depthstencil_attachment;

		GLuint depthRenderBuffer;
		pxGenRenderbuffers(1, &depthRenderBuffer);
		pxBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		pxRenderbufferStorage(GL_RENDERBUFFER, s_gl_internal_formats[(s32)internalFormat], swidth, sheight);
		pxFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
		pxBindRenderbuffer(GL_RENDERBUFFER, 0);

		upload_texture2d(depthTexture,
				swidth, sheight, texFormat, internalFormat, dataType, nullptr,
				filtering_e::linear, filtering_e::linear);
		detail::texture& depthTex = detail::s_textures[depthTexture];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex.gpu_handle, 0);
	}

	assert_framebuffer_completed();

	thisFramebuffer.gpu_handle = newFBO;
	thisFramebuffer.width = swidth;
	thisFramebuffer.height = sheight;
}

void setup_framebuffer(const framebuffer_handle_t i_hdl)
{
	if (i_hdl >= 0) {
		detail::framebuffer& thisFramebuffer = detail::s_framebuffers[i_hdl];
		pxBindFramebuffer(GL_FRAMEBUFFER, thisFramebuffer.gpu_handle);
		static GLenum drawBuffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3
		};
		pxDrawBuffers(thisFramebuffer.color_attachments_count, drawBuffers);
		pxViewport(0, 0, thisFramebuffer.width, thisFramebuffer.height);
	} else {
		pxBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void setup_framebuffer(const framebuffer_handle_t i_hdl, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	if (i_hdl >= 0) {
		detail::framebuffer& thisFramebuffer = detail::s_framebuffers[i_hdl];
		pxBindFramebuffer(GL_FRAMEBUFFER, thisFramebuffer.gpu_handle);
		static GLenum drawBuffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3
		};
		pxDrawBuffers(thisFramebuffer.color_attachments_count, drawBuffers);
		pxViewport(i_x, i_y, i_width, i_height);
	} else {
		pxBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

const texture_handle_t extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx)
{
	detail::framebuffer& thisFramebuffer = detail::s_framebuffers[i_fbHdl];
	return thisFramebuffer.color_attachments[i_idx];
}

const texture_handle_t extract_depth_stencil_attachment(const framebuffer_handle_t i_fbHdl)
{
	detail::framebuffer& thisFramebuffer = detail::s_framebuffers[i_fbHdl];
	return thisFramebuffer.depthstencil_attachment;
}

texture_handle_t create_texture()
{
	u32 idx = detail::s_textures.get_size();
	detail::s_textures.push_back(detail::texture());
	return (texture_handle_t)idx;
}

// we should gather all standalone 2d textures and upload them in one go
void upload_texture2d(const texture_handle_t& i_hdl,
		const s32 i_width, const s32 i_height,
		const texture_format_e i_format, const texture_internal_format_e i_internalFormat,
		const data_type_e i_dataType, voidptr i_data,
		const filtering_e i_minFil /* = filtering_e::nearest */, const filtering_e i_magFil /* = filtering_e::nearest */)
{
	detail::texture& thisTexture = detail::s_textures[i_hdl];
	GLuint newTexture = 0;
	
	pxGenTextures(1, &newTexture);
	pxBindTexture(GL_TEXTURE_2D, newTexture);

	// unpacking settings
	pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_filterings[static_cast<s32>(i_magFil)]);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_filterings[static_cast<s32>(i_minFil)]);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	pxTexImage2D(GL_TEXTURE_2D, 0,
			s_gl_internal_formats[static_cast<s32>(i_internalFormat)], i_width, i_height, 0,
			s_gl_texture_formats[static_cast<s32>(i_format)], s_gl_data_types[static_cast<s32>(i_dataType)], (GLvoid*)i_data);

	pxBindTexture(GL_TEXTURE_2D, 0);
	thisTexture.gpu_handle = newTexture;
	thisTexture.width = i_width;
	thisTexture.height = i_height;
	thisTexture.format = s_gl_texture_formats[static_cast<s32>(i_format)];
	thisTexture.internal_format = s_gl_internal_formats[static_cast<s32>(i_internalFormat)];
}

void upload_texture2d_mm(const texture_handle_t& i_hdl,
		const s32 i_width, const s32 i_height,
		const texture_format_e i_format, const texture_internal_format_e i_internalFormat,
		const data_type_e i_dataType, voidptr i_data,
		const filtering_e i_minFil /* = filtering_e::nearest */, const filtering_e i_magFil /* = filtering_e::nearest */)
{
	detail::texture& thisTexture = detail::s_textures[i_hdl];
	GLuint newTexture = 0;
	
	pxGenTextures(1, &newTexture);
	pxBindTexture(GL_TEXTURE_2D, newTexture);

	// unpacking settings
	pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s_filterings[static_cast<s32>(i_magFil)]);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s_filterings[static_cast<s32>(i_minFil)]);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	pxTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	s32 width = i_width;
	static size s_num_channels[] = {		// from i_internalFormat
		2, //GL_RG16F,
		3, //GL_RGB16F,
		4, //GL_RGBA16F,
		3, //GL_RGB8,
		4, //GL_RGBA8,
		3, //GL_SRGB8,
		4, //GL_SRGB8_ALPHA8,
		1, //GL_DEPTH_COMPONENT16,
		1, //GL_DEPTH_COMPONENT24,
		1 //GL_DEPTH24_STENCIL8
	};
	int mipIdx = 0;
	size offset = 0;
	while (width >= 1) {
		// NOTE: please remember that: when loading mipmaps, the width and height is
		// resolution of the mipmap, not the resolution of the largest mip
		pxTexImage2D(GL_TEXTURE_2D, mipIdx,
				s_gl_internal_formats[(s32)i_internalFormat], width, width, 0,
				s_gl_texture_formats[(s32)i_format], s_gl_data_types[(s32)i_dataType],
				(GLvoid*)((aptr)i_data + (aptr)offset));
		offset += width * width * s_num_channels[(s32)i_internalFormat];
		width >>= 1;
		mipIdx++;
	}

	pxBindTexture(GL_TEXTURE_2D, 0);
	thisTexture.gpu_handle = newTexture;
	thisTexture.width = i_width;
	thisTexture.height = i_height;
	thisTexture.format = s_gl_texture_formats[static_cast<s32>(i_format)];
	thisTexture.internal_format = s_gl_internal_formats[static_cast<s32>(i_internalFormat)];
}

void upload_texturecube(const texture_handle_t& i_hdl,
		const s32 i_width, const s32 i_height,
		const texture_format_e i_format, const texture_internal_format_e i_internalFormat,
		const data_type_e i_dataType, voidptr i_data,
		const filtering_e i_minFil /* = filtering_e::nearest */, const filtering_e i_magFil /* = filtering_e::nearest */)
{
	detail::texture& thisTexture = detail::s_textures[i_hdl];
	GLuint newTexture = 0;
	
	pxGenTextures(1, &newTexture);
	pxBindTexture(GL_TEXTURE_CUBE_MAP, newTexture);

	// unpacking settings
	pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, s_filterings[static_cast<s32>(i_magFil)]);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, s_filterings[static_cast<s32>(i_minFil)]);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	s32 width = i_width;
	static size s_num_channels[] = {		// from i_internalFormat
		2, //GL_RG16F,
		3, //GL_RGB16F,
		4, //GL_RGBA16F,
		3, //GL_RGB8,
		4, //GL_RGBA8,
		3, //GL_SRGB8,
		4, //GL_SRGB8_ALPHA8,
		1, //GL_DEPTH_COMPONENT16,
		1, //GL_DEPTH_COMPONENT24,
		1 //GL_DEPTH24_STENCIL8
	};

	size offset = 0;
	for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
		pxTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
				0,
				s_gl_internal_formats[(s32)i_internalFormat],
				i_width, i_width,
				0,
				s_gl_texture_formats[(s32)i_format],
				s_gl_data_types[(s32)i_dataType],
				(GLvoid*)((aptr)i_data + offset));
		offset += i_width * i_width * s_num_channels[(s32)i_internalFormat] * sizeof(f32);
	}
	//pxGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	pxBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	thisTexture.gpu_handle = newTexture;
	thisTexture.width = i_width;
	thisTexture.height = i_height;
	thisTexture.format = s_gl_texture_formats[static_cast<s32>(i_format)];
	thisTexture.internal_format = s_gl_internal_formats[static_cast<s32>(i_internalFormat)];
}

void upload_texturecube_mm(const texture_handle_t& i_hdl,
		const s32 i_width, const s32 i_height,
		const texture_format_e i_format, const texture_internal_format_e i_internalFormat,
		const data_type_e i_dataType, voidptr i_data,
		const filtering_e i_minFil /* = filtering_e::nearest */, const filtering_e i_magFil /* = filtering_e::nearest */)
{
	detail::texture& thisTexture = detail::s_textures[i_hdl];
	GLuint newTexture = 0;
	
	pxGenTextures(1, &newTexture);
	pxBindTexture(GL_TEXTURE_CUBE_MAP, newTexture);

	// unpacking settings
	pxPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	pxPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	pxPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	pxPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, s_filterings[static_cast<s32>(i_magFil)]);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, s_filterings[static_cast<s32>(i_minFil)]);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	pxTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	s32 width = i_width;
	static size s_num_channels[] = {		// from i_internalFormat
		2, //GL_RG16F,
		3, //GL_RGB16F,
		4, //GL_RGBA16F,
		3, //GL_RGB8,
		4, //GL_RGBA8,
		3, //GL_SRGB8,
		4, //GL_SRGB8_ALPHA8,
		1, //GL_DEPTH_COMPONENT16,
		1, //GL_DEPTH_COMPONENT24,
		1 //GL_DEPTH24_STENCIL8
	};
	int mipIdx = 0;
	size offset = 0;
	while (width >= 1) {
		// NOTE: please remember that: when loading mipmaps, the width and height is
		// resolution of the mipmap, not the resolution of the largest mip
		for (u32 faceIdx = 0; faceIdx < 6; faceIdx++) {
			pxTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
					mipIdx,
					s_gl_internal_formats[(s32)i_internalFormat],
					width, width,
					0,
					s_gl_texture_formats[(s32)i_format],
					s_gl_data_types[(s32)i_dataType],
					(GLvoid*)((aptr)i_data + (aptr)offset));
			offset += width * width * s_num_channels[(s32)i_internalFormat] * sizeof(f32);
		}
		width >>= 1;
		mipIdx++;
	}

	pxBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	thisTexture.gpu_handle = newTexture;
	thisTexture.width = i_width;
	thisTexture.height = i_height;
	thisTexture.format = s_gl_texture_formats[static_cast<s32>(i_format)];
	thisTexture.internal_format = s_gl_internal_formats[static_cast<s32>(i_internalFormat)];
}

shader_handle_t create_shader(const shader_param_list_t* i_paramList)
{
	u32 idx = detail::s_shaders.get_size();
	detail::s_shaders.push_back(detail::shader());
	material_template_t newTemplate;

	// create new material template first
	for (u32 i = 0; i < i_paramList->get_size(); i++) {
		const shader_param_t& thisParam = (*i_paramList)[i];
		floral::crc_string id(thisParam.name);
		switch (thisParam.data_type) {
			case param_data_type_e::param_float:
				{
					newTemplate.float_param_ids.push_back(id);
					break;
				}
			case param_data_type_e::param_vec3:
				{
					newTemplate.vec3_param_ids.push_back(id);
					break;
				}
			case param_data_type_e::param_mat4:
				{
					newTemplate.mat4_param_ids.push_back(id);
					break;
				}
			case param_data_type_e::param_sampler2d:
				{
					newTemplate.texture2d_param_ids.push_back(id);
					break;
				}
			case param_data_type_e::param_sampler_cube:
				{
					newTemplate.texturecube_param_ids.push_back(id);
					break;
				}
			case param_data_type_e::param_ub:
				{
					break;
				}
			default:
				break;
		}
	}
	detail::s_material_templates.push_back(newTemplate);

	return static_cast<shader_handle_t>(idx);
}

void compile_shader(shader_handle_t& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr)
{
	detail::shader& thisShader = detail::s_shaders[i_hdl];
	
	GLuint newShader = 0;
	GLuint fs = 0, vs = 0;
	newShader = pxCreateProgram();
	fs = pxCreateShader(GL_FRAGMENT_SHADER);
	vs = pxCreateShader(GL_VERTEX_SHADER);

	// compile vertex shader
	{
		GLint result = GL_FALSE;
		s32 infoLength;
		pxShaderSource(vs, 1, (const GLchar**)&i_vertstr, nullptr);
		pxCompileShader(vs);
		pxGetShaderiv(vs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(vs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Vertex Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	// compile fragment shader
	{
		GLint result = GL_FALSE;
		int infoLength;
		pxShaderSource(fs, 1, (const GLchar**)&i_fragstr, nullptr);
		pxCompileShader(fs);
		pxGetShaderiv(fs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(fs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Fragment Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	g_arena_allocator.free_all();

	// link them all!
	pxAttachShader(newShader, vs);
	pxAttachShader(newShader, fs);
	pxLinkProgram(newShader);

	// release temp shaders
	pxDeleteShader(vs);
	pxDeleteShader(fs);

	detail::s_shaders[i_hdl].gpu_handle = newShader;
}

void compile_shader(const shader_handle_t& i_hdl, const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList)
{
	CLOVER_DEBUG("Compiling shader: %d", i_hdl);
	detail::shader& thisShader = detail::s_shaders[i_hdl];
	
	GLuint newShader = 0;
	GLuint fs = 0, vs = 0;
	newShader = pxCreateProgram();
	fs = pxCreateShader(GL_FRAGMENT_SHADER);
	vs = pxCreateShader(GL_VERTEX_SHADER);

	// compile vertex shader
	{
		GLint result = GL_FALSE;
		s32 infoLength;
		pxShaderSource(vs, 1, (const GLchar**)&i_vertStr, nullptr);
		pxCompileShader(vs);
		pxGetShaderiv(vs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(vs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Vertex Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	// compile fragment shader
	{
		GLint result = GL_FALSE;
		int infoLength;
		pxShaderSource(fs, 1, (const GLchar**)&i_fragStr, nullptr);
		pxCompileShader(fs);
		pxGetShaderiv(fs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(fs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Fragment Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	g_arena_allocator.free_all();

	// link them all!
	pxAttachShader(newShader, vs);
	pxAttachShader(newShader, fs);
	pxLinkProgram(newShader);

	// release temp shaders
	pxDeleteShader(vs);
	pxDeleteShader(fs);

	// create symbol binding tables
	for (u32 i = 0; i < i_paramList->get_size(); i++) {
		const shader_param_t& thisParam = (*i_paramList)[i];
		GLuint id = pxGetUniformLocation(newShader, thisParam.name);
		switch (thisParam.data_type) {
			case param_data_type_e::param_float:
				{
					thisShader.float_params.push_back(id);
					break;
				}
			case param_data_type_e::param_vec3:
				{
					thisShader.vec3_params.push_back(id);
					break;
				}
			case param_data_type_e::param_mat4:
				{
					thisShader.mat4_params.push_back(id);
					break;
				}
			case param_data_type_e::param_sampler2d:
				{
					thisShader.texture2d_params.push_back(id);
					break;
				}
			case param_data_type_e::param_sampler_cube:
				{
					thisShader.texture_cube_params.push_back(id);
					break;
				}
			default:
				break;
		}
	}

	detail::s_shaders[i_hdl].gpu_handle = newShader;
}

const material_template_t& get_material_template(const shader_handle_t& i_shaderHdl)
{
	return detail::s_material_templates[static_cast<s32>(i_shaderHdl)];
}

surface_handle_t create_surface()
{
	u32 idx = detail::s_surfaces.get_size();
	detail::s_surfaces.push_back(detail::surface());
	return static_cast<surface_handle_t>(idx);
}

void upload_surface(surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices, const u32 i_vcount, const u32 i_icount, s32 i_stride, const draw_type_e i_drawType /* = draw_type_e::static_surface */)
{
	GLuint vbo = 0, ibo = 0;
	GLenum drawType = s_draw_types[static_cast<s32>(i_drawType)];
	pxGenBuffers(1, &vbo);
	pxBindBuffer(GL_ARRAY_BUFFER, vbo);
	pxBufferData(GL_ARRAY_BUFFER,
		(GLsizeiptr)(i_vcount * i_stride),
		(const GLvoid*)(i_vertices),
		drawType);
	pxBindBuffer(GL_ARRAY_BUFFER, 0);

	pxGenBuffers(1, &ibo);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	pxBufferData(GL_ELEMENT_ARRAY_BUFFER,
			(GLsizeiptr)(i_icount * sizeof(u32)),
			(const GLvoid*)(i_indices),
			drawType);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	detail::s_surfaces[i_hdl].stride = i_stride;
	detail::s_surfaces[i_hdl].draw_type = i_drawType;
	detail::s_surfaces[i_hdl].icount = i_icount;
	detail::s_surfaces[i_hdl].vbo = vbo;
	detail::s_surfaces[i_hdl].ibo = ibo;
}

void update_surface(const surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices,
		const u32 i_vcount, const u32 i_icount)
{
	s32 stride = detail::s_surfaces[i_hdl].stride;
	GLenum drawType = s_draw_types[static_cast<s32>(detail::s_surfaces[i_hdl].draw_type)];
	GLuint vbo = detail::s_surfaces[i_hdl].vbo;
	GLuint ibo = detail::s_surfaces[i_hdl].ibo;

	pxBindBuffer(GL_ARRAY_BUFFER, vbo);
	pxBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(i_vcount * stride),
			(const GLvoid*)(i_vertices),
			drawType);
	pxBindBuffer(GL_ARRAY_BUFFER, 0);

	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	pxBufferData(GL_ELEMENT_ARRAY_BUFFER,
			(GLsizeiptr)(i_icount * sizeof(u32)),
			(const GLvoid*)(i_indices),
			drawType);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	detail::s_surfaces[i_hdl].icount = i_icount;
}

}
}
