#include "insigne/ut_render.h"

#include <lotus/profiler.h>

#include "insigne/internal_states.h"
#include "insigne/counters.h"
#include "insigne/commands.h"
#include "insigne/ut_shading.h"
#include "insigne/ut_buffers.h"
#include "insigne/ut_textures.h"

namespace insigne {

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_render_allocator[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_render_command_buffer[detail::g_composing_cmdbuff];
}

// ---------------------------------------------
static inline void push_command(const render_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::render_command;
	newCmd.deserialize(i_cmd);

	get_composing_command_buffer().push_back(newCmd);
}

// render entrypoint----------------------------
void begin_frame()
{
	while (!detail::g_scene_presented.load());
	detail::g_scene_presented.store(false);
}

void end_frame()
{
}

void begin_render_pass(const framebuffer_handle_t i_fb)
{
	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_activate;
	cmd.framebuffer_activate_data.fb_handle = i_fb;
	cmd.framebuffer_activate_data.x = -1;
	cmd.framebuffer_activate_data.y = -1;
	cmd.framebuffer_activate_data.width = -1;
	cmd.framebuffer_activate_data.height = -1;

	push_command(cmd);
}

void begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	// scissor origin will be converted to top-left corner

	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_activate;
	cmd.framebuffer_activate_data.fb_handle = i_fb;
	cmd.framebuffer_activate_data.x = i_x;
	cmd.framebuffer_activate_data.y = i_y;
	cmd.framebuffer_activate_data.width = i_width;
	cmd.framebuffer_activate_data.height = i_height;

	push_command(cmd);
}

void end_render_pass(const framebuffer_handle_t i_fb)
{
	// TODO: perform some kinds of checking?
}

void mark_present_render()
{
	render_command_t cmd;
	cmd.command_type = render_command_type_e::present_render;

	push_command(cmd);
}

void dispatch_render_pass()
{
	detail::g_waiting_cmdbuffs.wait_and_push(detail::g_composing_cmdbuff);

	detail::g_composing_cmdbuff = (detail::g_composing_cmdbuff + 1) % BUFFERS_COUNT;
	cleanup_shading_module();
	cleanup_buffers_module();
	cleanup_textures_module();
	cleanup_render_module();
}

// ---------------------------------------------
framebuffer_desc_t create_framebuffer_desc()
{
	framebuffer_desc_t newDesc;
	newDesc.color_attachments = get_composing_allocator()->allocate<color_attachment_list_t>(MAX_COLOR_ATTACHMENTS, get_composing_allocator());
	newDesc.clear_color = floral::vec4f(0.0f, 0.0f, 0.0f, 0.0f);
	newDesc.width = 0;
	newDesc.height = 0;
	newDesc.scale = 1.0f;
	newDesc.has_depth = true;
	return newDesc;
}

const framebuffer_handle_t create_framebuffer(const framebuffer_desc_t& i_desc)
{
	framebuffer_handle_t newFbHdl = detail::create_framebuffer(i_desc);

	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_create;
	cmd.framebuffer_create_data.fb_handle = newFbHdl;
	cmd.framebuffer_create_data.desc = i_desc;

	push_command(cmd);
	return newFbHdl;
}

const texture_handle_t extract_color_attachment(const framebuffer_handle_t i_fb, const u32 i_idx)
{
	return detail::extract_color_attachment(i_fb, i_idx);
}

const texture_handle_t extract_depth_stencil_attachment(const framebuffer_handle_t i_fb)
{
	return detail::extract_depth_stencil_attachment(i_fb);
}

// ---------------------------------------------
void cleanup_render_module()
{
	// render allocator
	get_composing_allocator()->free_all();
	// draw call allocator
	detail::g_frame_draw_allocator[detail::g_composing_cmdbuff]->free_all();
}

}
