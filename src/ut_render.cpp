#include "insigne/ut_render.h"

namespace insigne {

// render entrypoint----------------------------
void begin_frame()
{
	PROFILE_SCOPE(begin_frame);

	{
		g_global_counters.current_submit_frame_idx++;
		memset(&g_debug_frame_counters, 0, sizeof(debug_frame_counters));
	}

	detail::s_waiting_for_swap = true;
}

void end_frame()
{
	PROFILE_SCOPE(end_frame);
	g_debug_global_counters.submitted_frames++;
	// wait for swap present here
	// spin spin spin spin
	while (detail::s_waiting_for_swap);
}

void begin_render_pass(const framebuffer_handle_t i_fb)
{
	PROFILE_SCOPE(begin_render_pass);
	// setup framebuffer
	{
		framebuffer_setup_command cmd;
		cmd.framebuffer_idx = i_fb;
		cmd.has_custom_viewport = false;
		gpu_command newGPUCmd;
		newGPUCmd.opcode = command::setup_framebuffer;
		newGPUCmd.deserialize(cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
	}
	// clear framebuffer
	{
		framebuffer_refresh_command cmd;
		cmd.region_clear = false;
		cmd.clear_color_buffer = true;
		cmd.clear_depth_buffer = true;
		gpu_command newGPUCmd;
		newGPUCmd.opcode = command::refresh_framebuffer;
		newGPUCmd.deserialize(cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
	}
}

void begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	PROFILE_SCOPE(begin_render_pass_viewport);
	// setup framebuffer
	{
		framebuffer_setup_command cmd;
		cmd.framebuffer_idx = i_fb;
		cmd.has_custom_viewport = true;
		cmd.lower_left_x = i_x; cmd.lower_left_y = i_y; cmd.width = i_width, cmd.height = i_height;
		gpu_command newGPUCmd;
		newGPUCmd.opcode = command::setup_framebuffer;
		newGPUCmd.deserialize(cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
	}
	// clear framebuffer
	{
		framebuffer_refresh_command cmd;
		cmd.region_clear = true;
		cmd.x = i_x; cmd.y = i_y; cmd.width = i_width; cmd.height = i_height;
		cmd.clear_color_buffer = true;
		cmd.clear_depth_buffer = true;
		gpu_command newGPUCmd;
		newGPUCmd.opcode = command::refresh_framebuffer;
		newGPUCmd.deserialize(cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
	}
}

void end_render_pass(const framebuffer_handle_t i_fb)
{
	// nothing, LUL
}

void mark_present_render()
{
	present_render_command cmd;
	cmd.placeholder = 1;
	gpu_command newGPUCmd;
	newGPUCmd.opcode = command::present_render;
	newGPUCmd.deserialize(cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
}

void dispatch_render_pass()
{
	{
		PROFILE_SCOPE(WaitingToDispatch);
		while ((detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES == detail::s_front_cmdbuff);
	}

	detail::s_back_cmdbuff = (detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES;
	{
		PROFILE_SCOPE(RenewComposingAllocator);
		s_composing_allocator.free_all();
		cleanup_shading_module();
		cleanup_buffers_module();
	}

	{
		PROFILE_SCOPE(NotifyRenderer);
		detail::s_cmdbuffer_condvar.notify_one();
	}
}
//framebuffer-----------------------------------
// deprecated
color_attachment_list_t* allocate_color_attachment_list(const u32 i_attachCount)
{
	return s_composing_allocator.allocate<color_attachment_list_t>(i_attachCount, &s_composing_allocator);
}

framebuffer_descriptor_t create_framebuffer_descriptor(const u32 i_colorAttachCount)
{
	framebuffer_descriptor_t retDesc;
	if (i_colorAttachCount > 0)
		retDesc.color_attachments = s_composing_allocator.allocate<color_attachment_list_t>(i_colorAttachCount, &s_composing_allocator);
	else
		retDesc.color_attachments = nullptr;
	retDesc.width = 0;
	retDesc.height = 0;
	retDesc.scale = 1.0f;
	retDesc.has_depth = false;
	return retDesc;
}

// deprecated
const framebuffer_handle_t create_framebuffer(const s32 i_width, const s32 i_height,
		const f32 i_scale, const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs)
{
	framebuffer_init_command cmd;
	cmd.color_attachment_list = i_colorAttachs;
	if (i_colorAttachs)
		cmd.framebuffer_idx = renderer::create_framebuffer(i_colorAttachs->get_size());
	else cmd.framebuffer_idx = renderer::create_framebuffer(0);
	cmd.width = i_width;
	cmd.height = i_height;
	cmd.scale = i_scale;
	cmd.has_depth = i_hasDepth;

	push_command(cmd);
	return cmd.framebuffer_idx;
}

const framebuffer_handle_t create_framebuffer(const framebuffer_descriptor_t& i_desc)
{
	framebuffer_init_command cmd;
	cmd.color_attachment_list = i_desc.color_attachments;
	if (i_desc.color_attachments)
		cmd.framebuffer_idx = renderer::create_framebuffer(i_desc.color_attachments->get_size());
	else cmd.framebuffer_idx = renderer::create_framebuffer(0);
	cmd.width = i_desc.width;
	cmd.height = i_desc.height;
	cmd.scale = i_desc.scale;
	cmd.has_depth = i_desc.has_depth;

	push_command(cmd);
	return cmd.framebuffer_idx;
}

const framebuffer_handle_t create_mega_framebuffer(const s32 i_width, const s32 i_height,
		const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs)
{
	framebuffer_init_command cmd;
	cmd.color_attachment_list = i_colorAttachs;
	cmd.framebuffer_idx = renderer::create_framebuffer(i_colorAttachs->get_size());
	cmd.width = i_width;
	cmd.height = i_height;
	cmd.scale = 1.0f;
	cmd.has_depth = i_hasDepth;

	push_command(cmd);
	return cmd.framebuffer_idx;
}

const texture_handle_t extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx)
{
	return renderer::extract_color_attachment(i_fbHdl, i_idx);
}

const texture_handle_t extract_depth_stencil_attachment(const framebuffer_handle_t i_fbHdl)
{
	return renderer::extract_depth_stencil_attachment(i_fbHdl);
}

}
