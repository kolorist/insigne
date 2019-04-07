#include "insigne/system.h"

#include <lotus/profiler.h>

#include <thread/mutex.h>
#include <thread/condition_variable.h>

#include <atomic>

#include "insigne/configs.h"
#include "insigne/counters.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/driver.h"
#include "insigne/detail/rt_shading.h"
#include "insigne/detail/rt_buffers.h"
#include "insigne/detail/rt_textures.h"
#include "insigne/detail/rt_render.h"

namespace insigne {

std::atomic_bool s_is_initialized(false);

bool s_resumed;
floral::mutex s_resumed_mtx;
floral::condition_variable s_resumed_cdv;

// ---------------------------------------------
void initialize_renderer()
{
	detail::initialize_shading_module();
	detail::initialize_buffers_module();
	detail::initialize_textures_module();
	detail::initialize_render_module();

	// do we really need this?
	GLuint vao;
	pxGenVertexArrays(1, &vao);
	pxBindVertexArray(vao);
}

void check_and_pause()
{
	floral::lock_guard guard(s_resumed_mtx);
	while (!s_resumed)
	{
		s_resumed_cdv.wait(s_resumed_mtx);
	}
}

void dispatch_frame()
{
	size toSubmitCmdBuff = -1;
	const bool hasCmdBuff = detail::g_waiting_cmdbuffs.try_pop_into(toSubmitCmdBuff);
	if (hasCmdBuff)
	{
		detail::process_shading_command_buffer(toSubmitCmdBuff);
		detail::process_buffers_command_buffer(toSubmitCmdBuff);
		detail::process_textures_command_buffer(toSubmitCmdBuff);

		bool swapBuffersThisPass = detail::process_render_command_buffer(toSubmitCmdBuff);
		detail::process_draw_command_buffer(toSubmitCmdBuff);

		if (swapBuffersThisPass) {
			swap_buffers();
			g_global_counters.current_frame_idx.fetch_add(1, std::memory_order_relaxed);
			detail::g_scene_presented.store(true);
		}
	}
	else
	{
		detail::g_is_dispatching.store(false, std::memory_order_relaxed);
	}
}

void render_thread_func(voidptr i_data)
{
	// profiler init
	lotus::init_capture_for_this_thread(1, "render_thread");
	create_main_context();
	initialize_renderer();
	s_is_initialized.store(true, std::memory_order_release);

	while (true)
	{
		while (!detail::g_is_dispatching.load(std::memory_order_acquire))
		{
			check_and_pause();
		}

		dispatch_frame();
	}
}

// ---------------------------------------------
void allocate_draw_command_buffers(const u32 i_maxSurfaceTypes)
{
	detail::g_draw_command_buffers.init(i_maxSurfaceTypes, &g_persistance_allocator);
}

void initialize_render_thread()
{
	g_global_counters.current_render_frame_idx = 0;
	g_global_counters.current_submit_frame_idx = 0;
	g_global_counters.current_frame_idx.store(0, std::memory_order_relaxed);
	g_debug_global_counters.submitted_frames = 0;
	g_debug_global_counters.rendered_frames = 0;

	// render
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_render_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_render_allocator_size_mb));
		detail::g_frame_draw_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_draw_allocator_size_mb));
		// TODO: hardcode!!!
		detail::g_render_command_buffer[i].init(16u, &g_persistance_allocator);
	}
	// shading
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_shader_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_shader_allocator_size_mb));
		// TODO: hardcode!!!
		detail::g_shading_command_buffer[i].init(16u, &g_persistance_allocator);
	}
	// buffers
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_buffers_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_buffers_allocator_size_mb));
		// TODO: hardcode!!!
		detail::g_buffers_command_buffer[i].init(128u, &g_persistance_allocator);
	}
	// textures
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_textures_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_textures_allocator_size_mb));
		// TODO: hardcode!!!
		detail::g_textures_command_buffer[i].init(128u, &g_persistance_allocator);
	}
	// ---

	detail::g_composing_cmdbuff = 0;
	detail::g_scene_presented = true;
	detail::g_context_dirty = true;

	detail::g_is_dispatching.store(false, std::memory_order_relaxed);
	s_is_initialized.store(false, std::memory_order_relaxed);
	{
		floral::lock_guard guard(s_resumed_mtx);
		s_resumed = false;
	}

	g_render_thread.entry_point = &insigne::render_thread_func;
	g_render_thread.ptr_data = nullptr;
	g_render_thread.start();
}

void pause_render_thread()
{
	floral::lock_guard guard(s_resumed_mtx);
	s_resumed = false;
}

void resume_render_thread()
{
	floral::lock_guard guard(s_resumed_mtx);
	s_resumed = true;
	s_resumed_cdv.notify_one();
}

void wait_for_initialization()
{
	while (!s_is_initialized.load(std::memory_order_acquire))
	{
	}
}

}
