#include "insigne/detail/rt_render.h"

#include "insigne/gl/identifiers.h"
#include "insigne/generated_code/proxy.h"

#include "insigne/detail/rt_shading.h"
#include "insigne/detail/rt_buffers.h"
#include "insigne/internal_states.h"

namespace insigne {
namespace detail {

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
// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_render_command_buffer() {
	return detail::g_render_command_buffer[detail::g_front_cmdbuff];
}

// ---------------------------------------------
const bool process_render_command_buffer()
{
	bool endOfFrameMarked = false;

	detail::gpu_command_buffer_t& cmdbuff = get_render_command_buffer();
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
