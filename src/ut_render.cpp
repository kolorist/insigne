#include "insigne/ut_render.h"

#include <lotus/profiler.h>

#include <floral/math/utils.h>

#include "insigne/internal_states.h"
#include "insigne/counters.h"
#include "insigne/commands.h"
#include "insigne/ut_shading.h"
#include "insigne/ut_buffers.h"
#include "insigne/ut_textures.h"

namespace insigne {

// ---------------------------------------------

struct render_resource_snapshot_t
{
	framebuffer_handle_t						handle;
};

static floral::inplace_array<render_resource_snapshot_t, 8>	s_resource_snapshots;

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator()
{
	return detail::g_frame_render_allocator[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer()
{
	return detail::g_render_command_buffer[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_draw_command_buffer(const ssize i_surfaceTypeIdx)
{
	return detail::g_draw_command_buffers[i_surfaceTypeIdx].command_buffer[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_post_draw_command_buffer(const ssize i_surfaceTypeIdx)
{
	return detail::g_post_draw_command_buffers[i_surfaceTypeIdx].command_buffer[detail::g_composing_cmdbuff];
}

// ---------------------------------------------
static inline void push_command(const render_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::render_command;
	newCmd.deserialize(i_cmd);

	get_composing_command_buffer().push_back(newCmd);
}

void push_draw_command(const ssize i_surfaceTypeIdx, const draw_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::draw_command;
	newCmd.deserialize(i_cmd);
	if (TEST_BIT(i_surfaceTypeIdx, 0x8000))
	{
		ssize surfaceTypeIdx = i_surfaceTypeIdx & 0x7FFF;
		get_composing_post_draw_command_buffer(surfaceTypeIdx).push_back(newCmd);
	}
	else
	{
		get_composing_draw_command_buffer(i_surfaceTypeIdx).push_back(newCmd);
	}
}

// render entrypoint----------------------------
void begin_frame()
{
	while (!detail::g_scene_presented.load() && !detail::g_waiting_cmdbuffs.is_empty());
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
	cmd.framebuffer_activate_data.face = cubemap_face_e::invalid;

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
	cmd.framebuffer_activate_data.face = cubemap_face_e::invalid;
	cmd.framebuffer_activate_data.to_mip = 0;

	push_command(cmd);
}

void begin_render_pass(const framebuffer_handle_t i_fb, const cubemap_face_e i_face)
{
	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_activate;
	cmd.framebuffer_activate_data.fb_handle = i_fb;
	cmd.framebuffer_activate_data.x = -1;
	cmd.framebuffer_activate_data.y = -1;
	cmd.framebuffer_activate_data.width = -1;
	cmd.framebuffer_activate_data.height = -1;
	cmd.framebuffer_activate_data.face = i_face;
	cmd.framebuffer_activate_data.to_mip = 0;

	push_command(cmd);
}

void begin_render_pass(const framebuffer_handle_t i_fb, const cubemap_face_e i_face, const s32 i_mip)
{
	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_activate;
	cmd.framebuffer_activate_data.fb_handle = i_fb;
	cmd.framebuffer_activate_data.x = -1;
	cmd.framebuffer_activate_data.y = -1;
	cmd.framebuffer_activate_data.width = -1;
	cmd.framebuffer_activate_data.height = -1;
	cmd.framebuffer_activate_data.face = i_face;
	cmd.framebuffer_activate_data.to_mip = i_mip;

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

const u64 schedule_framebuffer_capture(const framebuffer_handle_t i_fb, voidptr o_data)
{
	render_command_t cmd;
	cmd.command_type = render_command_type_e::framebuffer_capture;
	cmd.framebuffer_capture_data.fb_handle = i_fb;
	cmd.framebuffer_capture_data.pixel_data = o_data;

	push_command(cmd);
	return get_current_frame_idx() + 2;
}

// ---------------------------------------------
void cleanup_render_module()
{
	// render allocator
	get_composing_allocator()->free_all();
	// draw call allocator
	detail::g_frame_draw_allocator[detail::g_composing_cmdbuff]->free_all();
}

ssize get_render_resource_state()
{
	ssize stateId = s_resource_snapshots.get_size();
	render_resource_snapshot_t newSnapshot;
	newSnapshot.handle = detail::get_last_framebuffer();
	s_resource_snapshots.push_back(newSnapshot);
	return stateId;
}

void cleanup_render_resource(const ssize i_stateId)
{
	FLORAL_ASSERT_MSG(i_stateId == s_resource_snapshots.get_size() - 1, "Clean up render module does not according to snapshot order");
	const render_resource_snapshot_t snapShot = s_resource_snapshots.pop_back();

	render_command_t cmd;
	cmd.command_type = render_command_type_e::clean_up_snapshot;
	cmd.clean_up_snapshot_data.downto_handle = snapShot.handle;

	push_command(cmd);
}

}
