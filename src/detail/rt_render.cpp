#include "insigne/detail/rt_render.h"

#include "insigne/configs.h"
#include "insigne/commons.h"
#include "insigne/counters.h"
#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/detail/rt_shading.h"
#include "insigne/detail/rt_buffers.h"
#include "insigne/detail/rt_textures.h"
#include "insigne/internal_states.h"

#include <clover/Logger.h>

#include <lotus/profiler.h>

namespace insigne
{
namespace detail
{
// ------------------------------------------------------------------

framebuffers_pool_t								g_framebuffers_pool;

struct render_module_t
{
	render_state_t								render_state;
};
static render_module_t*							s_render_module = nullptr;

// ------------------------------------------------------------------
static const GLenum s_cmp_funcs[] = {
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static const GLenum s_front_faces[] = {
	GL_CW,
	GL_CCW
};

static const GLenum s_face_sides[] = {
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};

static const GLenum s_blend_equations[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

static const GLenum s_factors[] = {
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

static const GLenum s_stencil_ops[] = {
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_INCR_WRAP,
	GL_DECR,
	GL_DECR_WRAP,
	GL_INVERT
};

static const GLenum s_gl_data_types[] = {
	GL_UNSIGNED_BYTE,
	GL_BYTE,
	GL_UNSIGNED_INT,
	GL_INT,
	GL_FLOAT,
	GL_UNSIGNED_INT_24_8
};

static const GLenum s_polygon_modes[] = {
	GL_POINT,
	GL_LINE,
	GL_FILL
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
	pxFrontFace(s_front_faces[(s32)i_frontFace]);
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

void set_polygon_mode(const polygon_mode_e i_mode)
{
	pxPolygonMode(GL_FRONT_AND_BACK, s_polygon_modes[(s32)i_mode]);
}

// ------------------------------------------------------------------

static void setup_render_state(const render_state_t& i_renderState)
{
	// TODO: optimize me! reduce useless state change
	render_state_t& renderState = s_render_module->render_state;

	// depth
	if (i_renderState.depth_write != 2)
	{
		if (renderState.depth_write != i_renderState.depth_write)
		{
			renderState.depth_write = i_renderState.depth_write;
			if (renderState.depth_write == 1)
			{
				pxDepthMask(GL_TRUE);
			}
			else
			{
				pxDepthMask(GL_FALSE);
			}
		}
	}

	if (i_renderState.depth_test != 2)
	{
		if (renderState.depth_test != i_renderState.depth_test)
		{
			renderState.depth_test = i_renderState.depth_test;
			if (renderState.depth_test == 1)
			{
				pxEnable(GL_DEPTH_TEST);
			}
			else
			{
				pxDisable(GL_DEPTH_TEST);
			}
		}
	}

	if (i_renderState.depth_func != compare_func_e::func_undefined)
	{
		if (renderState.depth_func != i_renderState.depth_func)
		{
			renderState.depth_func = i_renderState.depth_func;
			pxDepthFunc(s_cmp_funcs[s32(renderState.depth_func)]);
		}
	}

	// face
	if (i_renderState.cull_face != 2)
	{
		if (renderState.cull_face != i_renderState.cull_face)
		{
			renderState.cull_face = i_renderState.cull_face;
			if (renderState.cull_face == 1)
			{
				pxEnable(GL_CULL_FACE);
			}
			else
			{
				pxDisable(GL_CULL_FACE);
			}
		}
	}

	if (i_renderState.face_side != face_side_e::undefined_side)
	{
		if (renderState.face_side != i_renderState.face_side)
		{
			renderState.face_side = i_renderState.face_side;
			pxCullFace(s_face_sides[s32(renderState.face_side)]);
		}
	}

	if (i_renderState.front_face != front_face_e::face_undefined)
	{
		if (renderState.front_face != i_renderState.front_face)
		{
			renderState.front_face = i_renderState.front_face;
			pxFrontFace(s_front_faces[s32(renderState.front_face)]);
		}
	}

	// blend
	if (i_renderState.blending != 2)
	{
		if (renderState.blending != i_renderState.blending)
		{
			renderState.blending = i_renderState.blending;
			if (renderState.blending == 1)
			{
				pxEnable(GL_BLEND);
			}
			else
			{
				pxDisable(GL_BLEND);
			}
		}
	}

	if (i_renderState.blend_equation != blend_equation_e::func_undefined)
	{
		if (renderState.blend_equation != i_renderState.blend_equation)
		{
			renderState.blend_equation = i_renderState.blend_equation;
			pxBlendEquation(s_blend_equations[s32(renderState.blend_equation)]);
		}
	}

	bool blendFuncChanged = false;
	if (i_renderState.blend_func_sfactor != factor_e::fact_undefined)
	{
		if (renderState.blend_func_sfactor != i_renderState.blend_func_sfactor)
		{
			renderState.blend_func_sfactor = i_renderState.blend_func_sfactor;
			blendFuncChanged = true;
		}
	}
	if (i_renderState.blend_func_dfactor != factor_e::fact_undefined)
	{
		if (renderState.blend_func_dfactor != i_renderState.blend_func_dfactor)
		{
			renderState.blend_func_dfactor = i_renderState.blend_func_dfactor;
			blendFuncChanged = true;
		}
	}
	if (blendFuncChanged)
	{
		pxBlendFunc(s_factors[s32(renderState.blend_func_sfactor)], s_factors[s32(renderState.blend_func_dfactor)]);
	}

	// TODO: add stencil
}

// ------------------------------------------------------------------

void enable_vertex_attrib(const u32 i_location)
{
	pxEnableVertexAttribArray(i_location);
}

void disable_vertex_attrib(const u32 i_location)
{
	pxDisableVertexAttribArray(i_location);
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
	ssize idx = g_framebuffers_pool.get_size();
	g_framebuffers_pool.push_back(framebuffer_desc_t());

	framebuffer_desc_t& desc = g_framebuffers_pool[idx];
	desc.clear_color = i_desc.clear_color;
	desc.width = i_desc.width;
	desc.height = i_desc.height;
	desc.scale = i_desc.scale;
	desc.has_depth = i_desc.has_depth;

	// attachments
	for (ssize i = 0; i < i_desc.color_attachments->get_size(); i++) {
		desc.color_attach_textures.push_back(create_texture());
		desc.color_attach_ids.push_back(floral::crc_string(i_desc.color_attachments->at(i).name));
	}
	if (i_desc.has_depth) {
		desc.depth_texture = create_texture();
	}

	return framebuffer_handle_t(idx);
}

/* ut */
const framebuffer_handle_t get_last_framebuffer()
{
	return g_framebuffers_pool.get_size() - 1;
}

/* ut */
const texture_handle_t extract_color_attachment(const framebuffer_handle_t i_fb, const u32 i_idx)
{
	framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_fb];

	return desc.color_attach_textures[i_idx];
}

/* ut */
const texture_handle_t extract_depth_stencil_attachment(const framebuffer_handle_t i_fb)
{
	framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_fb];

	return desc.depth_texture;
}

void initialize_framebuffer(const framebuffer_handle_t i_hdl, const insigne::framebuffer_desc_t& i_desc)
{
	framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];

	ssize colorAttachsCount = i_desc.color_attachments->get_size();

	GLuint newFBO = 0;
	pxGenFramebuffers(1, &newFBO);
	pxBindFramebuffer(GL_FRAMEBUFFER, newFBO);

	desc.gpu_handle = newFBO;

	s32 swidth = (s32)((f32)desc.width * desc.scale);
	s32 sheight = (s32)((f32)desc.height * desc.scale);

	for (ssize i = 0; i < colorAttachsCount; i++)
	{
		insigne::texture_desc_t colorDesc;
		colorDesc.data = nullptr;
		if (i_desc.color_attachments->at(i).width == 0 && i_desc.color_attachments->at(i).height == 0)
		{
			colorDesc.width = i_desc.width;
			colorDesc.height = i_desc.height;
		}
		else
		{
			colorDesc.width = i_desc.color_attachments->at(i).width;
			colorDesc.height = i_desc.color_attachments->at(i).height;
		}
		colorDesc.format = i_desc.color_attachments->at(i).texture_format;
		colorDesc.mag_filter = filtering_e::linear;
		colorDesc.has_mipmap = i_desc.color_has_mipmap;
		if (colorDesc.has_mipmap)
		{
			colorDesc.min_filter = filtering_e::linear_mipmap_linear;
		}
		else
		{
			colorDesc.min_filter = filtering_e::linear;
		}
		colorDesc.dimension = i_desc.color_attachments->at(i).texture_dimension;
		colorDesc.wrap_s = wrap_e::clamp_to_edge;
		colorDesc.wrap_t = wrap_e::clamp_to_edge;
		colorDesc.wrap_r = wrap_e::clamp_to_edge;
		upload_texture(desc.color_attach_textures[i], colorDesc);
		texture_desc_t& colorTex = g_textures_pool[(s32)desc.color_attach_textures[i]];
		if (colorDesc.dimension == texture_dimension_e::tex_2d)
		{
			pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (u32)i, GL_TEXTURE_2D, colorTex.gpu_handle, 0);
		}
		else
		{
			FLORAL_ASSERT(colorAttachsCount == 1);
			pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (u32)i, GL_TEXTURE_CUBE_MAP_POSITIVE_X, colorTex.gpu_handle, 0);
		}
	}

	if (desc.has_depth)
	{
		insigne::texture_desc_t depthDesc;
		depthDesc.data = nullptr;
		depthDesc.width = i_desc.width; depthDesc.height = i_desc.height;
		depthDesc.format = texture_format_e::depth;
		depthDesc.min_filter = filtering_e::linear; depthDesc.mag_filter = filtering_e::linear;
		depthDesc.dimension = texture_dimension_e::tex_2d;
		depthDesc.wrap_s = wrap_e::clamp_to_edge;
		depthDesc.wrap_t = wrap_e::clamp_to_edge;
		depthDesc.wrap_r = wrap_e::clamp_to_edge;
		depthDesc.has_mipmap = false;
		upload_texture(desc.depth_texture, depthDesc);

		texture_desc_t& depthTex = g_textures_pool[(s32)desc.depth_texture];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex.gpu_handle, 0);
	}

	assert_framebuffer_completed();
	pxBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void activate_framebuffer(const framebuffer_handle_t i_hdl, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height, const cubemap_face_e i_face, const s32 i_toMip)
{
	PROFILE_SCOPE("activate_framebuffer");
	static GLenum s_attachments[] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};

	if (i_face == cubemap_face_e::invalid)
	{
		if ((s32)i_hdl == DEFAULT_FRAMEBUFFER_HANDLE)
		{
			pxBindFramebuffer(GL_FRAMEBUFFER, 0);
			pxClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			set_scissor_test<false_type>(0, 0, 0, 0);
			pxViewport(0, 0, g_settings.native_res_x, g_settings.native_res_y);
			clear_framebuffer(true, true);
		}
		else
		{
			framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];
			pxBindFramebuffer(GL_FRAMEBUFFER, desc.gpu_handle);
			size numAttachments = desc.color_attach_textures.get_size();
			pxDrawBuffers((s32)numAttachments, s_attachments);
			if (i_width < 0 && i_height < 0) {
				set_scissor_test<false_type>(0, 0, 0, 0);
				pxViewport(0, 0, desc.width, desc.height);
			} else {
				set_scissor_test<true_type>(i_x, desc.height - i_y - i_height, i_width, i_height);
				pxViewport(i_x, desc.height - i_y - i_height, i_width, i_height);
			}
			pxClearColor(desc.clear_color.x, desc.clear_color.y, desc.clear_color.z, desc.clear_color.w);
			clear_framebuffer(true, desc.has_depth);
		}
	}
	else
	{
		static GLenum s_GLCubemapFaces[] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,		// invalid
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];
		pxBindFramebuffer(GL_FRAMEBUFFER, desc.gpu_handle);
		const texture_desc_t& colorTex = g_textures_pool[(s32)desc.color_attach_textures[0]];
		pxFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, s_GLCubemapFaces[(size)i_face], colorTex.gpu_handle, i_toMip);
		pxDrawBuffers(1, s_attachments);
		set_scissor_test<false_type>(0, 0, 0, 0);
		s32 width = desc.width >> i_toMip;
		s32 height = desc.height >> i_toMip;

		pxViewport(0, 0, width, height);
		pxClearColor(desc.clear_color.x, desc.clear_color.y, desc.clear_color.z, desc.clear_color.w);
		clear_framebuffer(true, desc.has_depth);
	}
}

void capture_framebuffer(const framebuffer_handle_t i_hdl, voidptr o_data)
{
	// TODO: async reading
	const framebuffer_desc_t& desc = g_framebuffers_pool[(s32)i_hdl];
	const texture_desc_t& texDesc = g_textures_pool[(s32)desc.color_attach_textures[0]];
	GLint oldFb;
	pxGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldFb);
	static GLenum s_GLReadFormat[] = {
		GL_RED,									// r
		GL_RG,									// rg
		GL_RG,									// hdr_rg
		GL_RGB,									// rgb
		GL_RGB,									// hdr_rgb
		GL_RGB,									// hdr_rgb_half
		GL_RGB,									// srgb
		GL_RGBA,								// srgba
		GL_RGBA,								// rgba
		GL_RGBA,								// hdr_rgba
		GL_RED,									// depth
		GL_RED									// depth_stencil
	};
	if (texDesc.dimension == texture_dimension_e::tex_2d)
	{
		pxBindFramebuffer(GL_READ_FRAMEBUFFER, desc.gpu_handle);
		pxReadBuffer(GL_COLOR_ATTACHMENT0);
		pxReadPixels(0, 0, desc.width, desc.height, s_GLReadFormat[(s32)texDesc.format], GL_FLOAT, o_data);
	}
	else if (texDesc.dimension == texture_dimension_e::tex_cube)
	{
		FLORAL_ASSERT(desc.width == desc.height);
		FLORAL_ASSERT(texDesc.width == texDesc.height);
		pxBindFramebuffer(GL_READ_FRAMEBUFFER, desc.gpu_handle);
		pxReadBuffer(GL_COLOR_ATTACHMENT0);
		static GLenum s_GLCubemapFaces[] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};
		f32* oData = (f32*)o_data;
		for (s32 i = 0; i < 6; i++)
		{
			s32 texSize = texDesc.width;
			s32 mipIdx = 0;
			while (texSize >= 1)
			{
				pxFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						s_GLCubemapFaces[i], texDesc.gpu_handle, mipIdx);
				pxReadPixels(0, 0, texSize, texSize, s_GLReadFormat[(s32)texDesc.format], GL_FLOAT, oData);
				// TODO: fix channel count hardcode
				oData += texSize * texSize * 3;
				texSize >>= 1;
				mipIdx++;
			}
		}
	}
	pxBindFramebuffer(GL_READ_FRAMEBUFFER, oldFb);
}

void draw_indexed_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t* i_mat,
		const s32 i_segSize, const s32 i_segOffset, geometry_mode_e i_geometryMode,
		states_setup_func_t i_stateSetup, vertex_data_setup_func_t i_vertexSetup)
{
	//i_stateSetup();
	setup_render_state(i_mat->render_state);
	const insigne::detail::vbdesc_t& vbDesc = insigne::detail::g_vbs_pool[s32(i_vb)];
	const insigne::detail::ibdesc_t& ibDesc = insigne::detail::g_ibs_pool[s32(i_ib)];
	const insigne::detail::shader_desc_t& shaderDesc = insigne::detail::g_shaders_pool[s32(i_mat->shader_handle)];

	pxUseProgram(shaderDesc.gpu_handle);

	for (u32 i = 0; i < i_mat->textures.get_size(); i++) {
		texture_handle_t texId = i_mat->textures[i].value;
		if (texId < 0) continue;
		const insigne::detail::texture_desc_t& texDesc = insigne::detail::g_textures_pool[texId];
		pxActiveTexture(GL_TEXTURE0 + i);
		if (texDesc.dimension == texture_dimension_e::tex_2d)
		{
			pxBindTexture(GL_TEXTURE_2D, texDesc.gpu_handle);
		}
		if (texDesc.dimension == texture_dimension_e::tex_3d)
		{
			pxBindTexture(GL_TEXTURE_3D, texDesc.gpu_handle);
		}
		else if (texDesc.dimension == texture_dimension_e::tex_cube)
		{
			pxBindTexture(GL_TEXTURE_CUBE_MAP, texDesc.gpu_handle);
		}
		pxUniform1i(shaderDesc.slots_config.textures[i], i);
	}

	for (u32 i = 0; i < i_mat->uniform_blocks.get_size(); i++) {
		const ubmat_desc_t ubMatDesc = i_mat->uniform_blocks[i].value;
		if (ubMatDesc.ub_handle < 0) continue;
		if (shaderDesc.slots_config.uniform_blocks[i] == GL_INVALID_INDEX) continue;
		const insigne::detail::ubdesc_t& ubDesc = insigne::detail::g_ubs_pool[ubMatDesc.ub_handle];
		if (ubMatDesc.offset == 0)
		{
			pxBindBufferBase(GL_UNIFORM_BUFFER, i, ubDesc.gpu_handle);
		}
		else
		{
			pxBindBufferRange(GL_UNIFORM_BUFFER, i, ubDesc.gpu_handle,
					ubMatDesc.offset * ubDesc.alignment,
					ubMatDesc.range * ubDesc.alignment);
		}
		pxUniformBlockBinding(shaderDesc.gpu_handle, shaderDesc.slots_config.uniform_blocks[i], i);
	}

	// draw
	static GLenum s_geometryMode[] = {
		GL_POINTS,
		GL_LINE_STRIP,
		GL_LINE_LOOP,
		GL_LINES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
		GL_TRIANGLES
	};

	pxBindBuffer(GL_ARRAY_BUFFER, vbDesc.gpu_handle);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibDesc.gpu_handle);
	i_vertexSetup();
	if (i_segOffset == 0 && i_segSize == 0)
	{
		pxDrawElements(s_geometryMode[(s32)i_geometryMode], ibDesc.count, GL_UNSIGNED_INT, 0);
	}
	else
	{
		pxDrawElements(s_geometryMode[(s32)i_geometryMode], i_segSize, GL_UNSIGNED_INT, (voidptr)(i_segOffset * sizeof(s32)));
	}
}


// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_render_command_buffer(const size i_cmdBuffId) {
	return detail::g_render_command_buffer[i_cmdBuffId];
}

// ---------------------------------------------
void initialize_render_module()
{
	// create default framebuffer desc
	FLORAL_ASSERT(s_render_module == nullptr);
	s_render_module = g_persistance_allocator.allocate<render_module_t>();

	render_state_t& renderState = s_render_module->render_state;

	renderState.depth_test = 1;
	renderState.depth_write = 1;
	renderState.depth_func = compare_func_e::func_less;
	pxEnable(GL_DEPTH_TEST);
	pxDepthMask(GL_TRUE);
	pxDepthFunc(GL_LESS);

	renderState.cull_face = 1;
	renderState.face_side = face_side_e::back_side;
	renderState.front_face = front_face_e::face_ccw;
	pxEnable(GL_CULL_FACE);
	pxCullFace(GL_BACK);
	pxFrontFace(GL_CCW);

	renderState.blending = 0;
	renderState.blend_equation = blend_equation_e::func_add;
	renderState.blend_func_sfactor = factor_e::fact_one;
	renderState.blend_func_dfactor = factor_e::fact_zero;
	pxDisable(GL_BLEND);
	pxBlendEquation(GL_FUNC_ADD);
	pxBlendFunc(GL_ONE, GL_ZERO);

	renderState.stencil_test = 0;
	renderState.stencil_func = compare_func_e::func_always;
	renderState.stencil_mask = 0xFFFFFFFF;
	renderState.stencil_ref = 0;
	renderState.stencil_op_sfail = operation_e::oper_keep;
	renderState.stencil_op_dpfail = operation_e::oper_keep;
	renderState.stencil_op_dppass = operation_e::oper_keep;
	pxDisable(GL_STENCIL_TEST);
	pxStencilFunc(GL_ALWAYS, renderState.stencil_ref, renderState.stencil_mask);
	pxStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

// ---------------------------------------------

static void clean_up_snapshot(const framebuffer_handle_t i_fb)
{
	CLOVER_VERBOSE("Cleaning up render buffers snapshot...");
	while (i_fb != g_framebuffers_pool.get_size() - 1)
	{
		framebuffer_desc_t framebufferDesc = g_framebuffers_pool.pop_back();
		CLOVER_VERBOSE("Deleting framebuffer id %d", framebufferDesc.gpu_handle);
		pxDeleteFramebuffers(1, &framebufferDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Finished cleaning up render buffers snapshot");
}

// ---------------------------------------------
void cleanup_render_module()
{
	CLOVER_VERBOSE("Cleaning up render module...");
	for (ssize i = 0; i < g_framebuffers_pool.get_size(); i++)
	{
		framebuffer_desc_t& framebufferDesc = g_framebuffers_pool[i];
		CLOVER_VERBOSE("Deleting framebuffer id %d", framebufferDesc.gpu_handle);
		pxDeleteFramebuffers(1, &framebufferDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd frame buffers", g_framebuffers_pool.get_size());
	s_render_module = nullptr;
	CLOVER_VERBOSE("Finished cleaning up render module...");
}

// ---------------------------------------------
void process_draw_command_buffer(const size i_cmdBuffId)
{
	PROFILE_SCOPE("process_draw_command_buffer");
	floral::lock_guard guard(detail::g_draw_config_mtx);

	u64 writeSlot = g_global_counters.current_write_slot;

	// geometry render phase
	for (u32 i = 0; i < g_settings.surface_types_count; i++)
	{
		detail::gpu_command_buffer_t& cmdbuff = g_draw_command_buffers[i].command_buffer[i_cmdBuffId];
		states_setup_func_t stateSetupFunc = g_draw_command_buffers[i].states_setup_func;
		vertex_data_setup_func_t vertexSetupFunc = g_draw_command_buffers[i].vertex_data_setup_func;
		geometry_mode_e geometryMode = g_draw_command_buffers[i].geometry_mode;

		g_debug_frame_counters[writeSlot].num_draw_commands += (u32)cmdbuff.get_size();

		for (u32 j = 0; j < cmdbuff.get_size(); j++) {
			gpu_command& gpuCmd = cmdbuff[j];
			gpuCmd.reset_cursor();
			switch (gpuCmd.opcode) {
				case command::draw_command:
					{
						draw_command_t cmd;
						gpuCmd.serialize(cmd);

						switch (cmd.command_type) {
							case draw_command_type_e::draw_surface:
							{
								detail::draw_indexed_surface(
										cmd.draw_surface_data.vb_handle,
										cmd.draw_surface_data.ib_handle,
										cmd.draw_surface_data.material_snapshot,
										cmd.draw_surface_data.segment_size,
										cmd.draw_surface_data.segment_offset,
										geometryMode,
										stateSetupFunc,
										vertexSetupFunc);
								break;
							}
							case draw_command_type_e::state_setup_scissor:
							{
								if (cmd.state_setup_scissor_data.enabled) {
									detail::set_scissor_test<true_type>(
											cmd.state_setup_scissor_data.x,
											cmd.state_setup_scissor_data.y,
											cmd.state_setup_scissor_data.width,
											cmd.state_setup_scissor_data.height);
								} else {
									detail::set_scissor_test<false_type>(0, 0, 0, 0);
								}
								break;
							}
						}
					}
				default:
					break;
			}
		}
	}

	// cleaning up
	for (u32 i = 0; i < g_settings.surface_types_count; i++)
	{
		detail::gpu_command_buffer_t& cmdBuff = g_draw_command_buffers[i].command_buffer[i_cmdBuffId];
		cmdBuff.clear();
	}
}

void process_post_draw_command_buffer(const size i_cmdBuffId)
{
	PROFILE_SCOPE("process_post_draw_command_buffer");
	floral::lock_guard guard(detail::g_post_draw_config_mtx);
	// geometry render phase

	u64 writeSlot = g_global_counters.current_write_slot;

	for (u32 i = 0; i < g_settings.post_surface_types_count; i++)
	{
		detail::gpu_command_buffer_t& cmdbuff = g_post_draw_command_buffers[i].command_buffer[i_cmdBuffId];
		states_setup_func_t stateSetupFunc = g_post_draw_command_buffers[i].states_setup_func;
		vertex_data_setup_func_t vertexSetupFunc = g_post_draw_command_buffers[i].vertex_data_setup_func;
		geometry_mode_e geometryMode = g_post_draw_command_buffers[i].geometry_mode;

		g_debug_frame_counters[writeSlot].num_draw_commands += (u32)cmdbuff.get_size();

		for (u32 j = 0; j < cmdbuff.get_size(); j++) {
			gpu_command& gpuCmd = cmdbuff[j];
			gpuCmd.reset_cursor();
			switch (gpuCmd.opcode) {
				case command::draw_command:
					{
						draw_command_t cmd;
						gpuCmd.serialize(cmd);

						switch (cmd.command_type) {
							case draw_command_type_e::draw_surface:
							{
								detail::draw_indexed_surface(
										cmd.draw_surface_data.vb_handle,
										cmd.draw_surface_data.ib_handle,
										cmd.draw_surface_data.material_snapshot,
										cmd.draw_surface_data.segment_size,
										cmd.draw_surface_data.segment_offset,
										geometryMode,
										stateSetupFunc,
										vertexSetupFunc);
								break;
							}
							case draw_command_type_e::state_setup_scissor:
							{
								if (cmd.state_setup_scissor_data.enabled) {
									detail::set_scissor_test<true_type>(
											cmd.state_setup_scissor_data.x,
											cmd.state_setup_scissor_data.y,
											cmd.state_setup_scissor_data.width,
											cmd.state_setup_scissor_data.height);
								} else {
									detail::set_scissor_test<false_type>(0, 0, 0, 0);
								}
								break;
							}
						}
					}
				default:
					break;
			}
		}
	}

	// cleaning up
	for (u32 i = 0; i < g_settings.post_surface_types_count; i++)
	{
		detail::gpu_command_buffer_t& cmdBuff = g_post_draw_command_buffers[i].command_buffer[i_cmdBuffId];
		cmdBuff.clear();
	}
}

const bool process_render_command_buffer(const size i_cmdBuffId)
{
	PROFILE_SCOPE("process_render_command_buffer");
	bool endOfFrameMarked = false;
	detail::gpu_command_buffer_t& cmdbuff = get_render_command_buffer(i_cmdBuffId);

	u64 writeSlot = g_global_counters.current_write_slot;
	g_debug_frame_counters[writeSlot].num_render_commands += (u32)cmdbuff.get_size();

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
									cmd.framebuffer_activate_data.height,
									cmd.framebuffer_activate_data.face,
									cmd.framebuffer_activate_data.to_mip);
							break;
						case render_command_type_e::framebuffer_capture:
							capture_framebuffer(cmd.framebuffer_capture_data.fb_handle,
									cmd.framebuffer_capture_data.pixel_data);
							break;
						case render_command_type_e::clean_up_snapshot:
							clean_up_snapshot(cmd.clean_up_snapshot_data.downto_handle);
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

	cmdbuff.clear();

	return endOfFrameMarked;
}

// ------------------------------------------------------------------
}
}
