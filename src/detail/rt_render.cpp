#include "insigne/detail/rt_render.h"

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/detail/rt_shading.h"
#include "insigne/detail/rt_buffers.h"
#include "insigne/detail/rt_textures.h"
#include "insigne/internal_states.h"

namespace insigne {
namespace detail {

framebuffers_pool_t								g_framebuffers_pool;

// -----------------------------------------
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

static GLenum s_gl_data_types[] = {
	GL_UNSIGNED_BYTE,
	GL_BYTE,
	GL_UNSIGNED_INT,
	GL_INT,
	GL_FLOAT,
	GL_UNSIGNED_INT_24_8
};

// -----------------------------------------
void clear_color(const floral::vec4f& i_color)
{
	pxClearColor(i_color.x, i_color.y, i_color.z, i_color.w);
}

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

// ---------------------------------------------
/* ut */
const framebuffer_handle_t create_framebuffer(const insigne::framebuffer_desc_t& i_desc)
{
	u32 idx = g_framebuffers_pool.get_size();
	g_framebuffers_pool.push_back(framebuffer_desc_t());

	framebuffer_desc_t& desc = g_framebuffers_pool[idx];
	desc.clear_color = i_desc.clear_color;
	desc.width = i_desc.width;
	desc.height = i_desc.height;
	desc.scale = i_desc.scale;
	desc.has_depth = i_desc.has_depth;

	// attachments
	for (u32 i = 0; i < i_desc.color_attachments->get_size(); i++) {
		desc.color_attach_textures.push_back(create_texture());
		desc.color_attach_ids.push_back(floral::crc_string(i_desc.color_attachments->at(i).name));
	}
	if (i_desc.has_depth) {
		desc.depth_texture = create_texture();
	}

	return framebuffer_handle_t(idx);
}

void initialize_framebuffer(const framebuffer_handle_t i_hdl, const insigne::framebuffer_desc_t& i_desc)
{
	framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];

	u32 colorAttachsCount = i_desc.color_attachments->get_size();

	GLuint newFBO = 0;
	pxGenFramebuffers(1, &newFBO);
	pxBindFramebuffer(GL_FRAMEBUFFER, newFBO);

	desc.gpu_handle = newFBO;

	s32 swidth = (s32)((f32)desc.width * desc.scale);
	s32 sheight = (s32)((f32)desc.height * desc.scale);

	for (u32 i = 0; i < colorAttachsCount; i++) {
		insigne::texture_desc_t colorDesc;
		colorDesc.data = nullptr;
		colorDesc.width = i_desc.width; colorDesc.height = i_desc.height;
		colorDesc.format = i_desc.color_attachments->at(i).texture_format;
		colorDesc.min_filter = filtering_e::linear; colorDesc.mag_filter = filtering_e::linear;
		colorDesc.dimension = texture_dimension_e::tex_2d;
		colorDesc.has_mipmap = false;
		upload_texture(desc.color_attach_textures[i], colorDesc);
		// attach texture to fbo
		texture_desc_t& colorTex = g_textures_pool[(s32)desc.color_attach_textures[i]];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTex.gpu_handle, 0);
	}

	if (desc.has_depth)
	{
		insigne::texture_desc_t depthDesc;
		depthDesc.data = nullptr;
		depthDesc.width = i_desc.width; depthDesc.height = i_desc.height;
		depthDesc.format = texture_format_e::depth;
		depthDesc.min_filter = filtering_e::linear; depthDesc.mag_filter = filtering_e::linear;
		depthDesc.dimension = texture_dimension_e::tex_2d;
		depthDesc.has_mipmap = false;

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

		GLuint depthRenderBuffer;
		pxGenRenderbuffers(1, &depthRenderBuffer);
		pxBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		pxRenderbufferStorage(GL_RENDERBUFFER, s_GLInternalFormat[(s32)depthDesc.format], swidth, sheight);
		pxFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
		pxBindRenderbuffer(GL_RENDERBUFFER, 0);

		upload_texture(desc.depth_texture, depthDesc);

		texture_desc_t& depthTex = g_textures_pool[(s32)desc.depth_texture];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex.gpu_handle, 0);
	}

	assert_framebuffer_completed();
	pxBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void activate_framebuffer(const framebuffer_handle_t i_hdl, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	if ((s32)i_hdl == DEFAULT_FRAMEBUFFER_HANDLE) {
		pxBindFramebuffer(GL_FRAMEBUFFER, 0);
		pxClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		clear_framebuffer(true, true);
	} else {
		framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];
		pxBindFramebuffer(GL_FRAMEBUFFER, desc.gpu_handle);
		if (i_width < 0 && i_height < 0) {
			set_scissor_test<false_type>(0, 0, 0, 0);
		} else {
			set_scissor_test<true_type>(i_x, desc.height - i_y, i_width, i_height);
		}
		pxClearColor(desc.clear_color.x, desc.clear_color.y, desc.clear_color.z, desc.clear_color.w);
		clear_framebuffer(true, desc.has_depth);
	}
}

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_render_command_buffer(const size i_cmdBuffId) {
	return detail::g_render_command_buffer[i_cmdBuffId];
}

// ---------------------------------------------
const bool process_render_command_buffer(const size i_cmdBuffId)
{
	bool endOfFrameMarked = false;
	detail::gpu_command_buffer_t& cmdbuff = get_render_command_buffer(i_cmdBuffId);
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::render_command:
				{
					render_command_t cmd;
					gpuCmd.serialize(cmd);

					switch (cmd.command_type) {
						case render_command_type_e::present_render:
							endOfFrameMarked = true;
							break;
						case render_command_type_e::framebuffer_create:
							initialize_framebuffer(cmd.framebuffer_create_data.fb_handle, cmd.framebuffer_create_data.desc);
							break;
						case render_command_type_e::framebuffer_activate:
							activate_framebuffer(cmd.framebuffer_activate_data.fb_handle,
									cmd.framebuffer_activate_data.x,
									cmd.framebuffer_activate_data.y,
									cmd.framebuffer_activate_data.width,
									cmd.framebuffer_activate_data.height);
							break;
						default:
							break;
					}
					break;
				}
			default:
				break;
		}
	}

	cmdbuff.empty();

	return endOfFrameMarked;
}

}
}
