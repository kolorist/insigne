#include "insigne/system.h"

#include "insigne/configs.h"
#include "insigne/counters.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/driver.h"
#include "insigne/detail/rt_shading.h"
#include "insigne/detail/rt_buffers.h"
#include "insigne/detail/rt_textures.h"
#include "insigne/detail/rt_render.h"

#include <clover/Logger.h>

#include <lotus/profiler.h>

#include <floral/thread/mutex.h>
#include <floral/thread/condition_variable.h>

#include <atomic>

namespace insigne
{
//----------------------------------------------

static std::atomic_bool s_is_initialized(false);

static bool s_resumed;
static floral::mutex s_resumed_mtx;
static floral::condition_variable s_resumed_cdv;

static std::atomic_bool s_stopped(false);
static GLuint s_vao;

//----------------------------------------------

static void initialize_renderer()
{
	detail::initialize_shading_module();
	detail::initialize_buffers_module();
	detail::initialize_textures_module();
	detail::initialize_render_module();

	// do we really need this?
	pxGenVertexArrays(1, &s_vao);
	pxBindVertexArray(s_vao);

	// enable interpolating across cubemap's faces
	pxEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// disable sRGB for main buffers
	pxBindFramebuffer(GL_FRAMEBUFFER, 0);
	pxDisable(GL_FRAMEBUFFER_SRGB);
}

//----------------------------------------------

static void cleanup_renderer()
{
	pxDeleteVertexArrays(1, &s_vao);

	detail::cleanup_render_module();
	detail::cleanup_textures_module();
	detail::cleanup_buffers_module();
	detail::cleanup_shading_module();
}

//----------------------------------------------

void check_and_pause()
{
	floral::lock_guard guard(s_resumed_mtx);
	while (!s_resumed)
	{
		CLOVER_VERBOSE("Render thread paused");
		s_resumed_cdv.wait(s_resumed_mtx);
		insigne::refresh_context();
		CLOVER_VERBOSE("Render thread resumed");
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
		detail::process_post_draw_command_buffer(toSubmitCmdBuff);

		if (swapBuffersThisPass) {
			swap_buffers();
			g_global_counters.current_frame_idx.fetch_add(1, std::memory_order_relaxed);
			detail::g_scene_presented.store(true);
		}
	}
}

	// profiler init
void render_thread_func(voidptr i_data)
{
	lotus::init_capture_for_this_thread(1, "render_thread");

	CLOVER_INIT_THIS_THREAD("render_thread", clover::LogLevel::Verbose);
	CLOVER_VERBOSE("Render Thread started");

	create_main_context();
	initialize_renderer();
	s_is_initialized.store(true, std::memory_order_release);

	while (s_stopped.load(std::memory_order_acquire) != true)
	{
		while (detail::g_waiting_cmdbuffs.is_empty())
		{
			if (s_stopped.load(std::memory_order_relaxed))
			{
				break;
			}
			check_and_pause();
		}

		dispatch_frame();
	}

	cleanup_renderer();
	s_is_initialized.store(false, std::memory_order_release);

	CLOVER_VERBOSE("Render Thread stopped");
}

// ---------------------------------------------

void organize_memory()
{
	// draw buffers partition
	FLORAL_ASSERT_MSG(detail::g_draw_cmdbuff_arena == nullptr, "Already initialize g_draw_cmdbuff_arena");
	FLORAL_ASSERT_MSG(detail::g_post_draw_cmdbuff_arena == nullptr, "Already initialize g_post_draw_cmdbuff_arena");
	FLORAL_ASSERT_MSG(g_settings.draw_cmdbuff_arena_size_mb > 0, "Invalid size for g_draw_cmdbuff_arena");
	FLORAL_ASSERT_MSG(g_settings.post_draw_cmdbuff_arena_size_mb > 0, "Invalid size for g_post_draw_cmdbuff_arena");
	detail::g_draw_cmdbuff_arena = g_persistance_allocator.allocate_arena<linear_allocator_t>(
			SIZE_MB(g_settings.draw_cmdbuff_arena_size_mb));
	detail::g_post_draw_cmdbuff_arena = g_persistance_allocator.allocate_arena<linear_allocator_t>(
			SIZE_MB(g_settings.post_draw_cmdbuff_arena_size_mb));

	// scene memory partition
	FLORAL_ASSERT_MSG(g_scene_settings.max_shaders > 0, "Invalid max shaders config");
	FLORAL_ASSERT_MSG(g_scene_settings.max_ubos > 0, "Invalid max UBO config");
	FLORAL_ASSERT_MSG(g_scene_settings.max_ibos > 0, "Invalid max IBO config");
	FLORAL_ASSERT_MSG(g_scene_settings.max_vbos > 0, "Invalid max VBO config");
	FLORAL_ASSERT_MSG(g_scene_settings.max_textures > 0, "Invalid max textures config");
	FLORAL_ASSERT_MSG(g_scene_settings.max_fbos > 0, "Invalid max FBO config");
	FLORAL_ASSERT_MSG(detail::g_resource_arena == nullptr, "Already initialize g_resource_arena");

	size requiredMemory =
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_shaders * sizeof(detail::shader_desc_t)) +
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_ubos * sizeof(detail::ubdesc_t)) +
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_ibos * sizeof(detail::ibdesc_t)) +
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_vbos * sizeof(detail::vbdesc_t)) +
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_textures * sizeof(detail::texture_desc_t)) +
		helich::stack_scheme<helich::no_tracking_policy>::get_real_data_size(g_scene_settings.max_fbos * sizeof(detail::framebuffer_desc_t));
	detail::g_resource_arena = g_persistance_allocator.allocate_arena<linear_allocator_t>(requiredMemory);
	detail::g_shaders_pool.reserve(g_scene_settings.max_shaders, detail::g_resource_arena);
	detail::g_vbs_pool.reserve(g_scene_settings.max_vbos, detail::g_resource_arena);
	detail::g_ibs_pool.reserve(g_scene_settings.max_ibos, detail::g_resource_arena);
	detail::g_ubs_pool.reserve(g_scene_settings.max_ubos, detail::g_resource_arena);
	detail::g_textures_pool.reserve(g_scene_settings.max_textures, detail::g_resource_arena);
	detail::g_framebuffers_pool.reserve(g_scene_settings.max_fbos, detail::g_resource_arena);
}

void allocate_draw_command_buffers(const u32 i_maxSurfaceTypes)
{
	FLORAL_ASSERT(detail::g_draw_cmdbuff_arena != nullptr);
	detail::g_draw_command_buffers.reserve(i_maxSurfaceTypes, detail::g_draw_cmdbuff_arena);
}

static void free_all_draw_command_buffers()
{
	floral::free_array(detail::g_draw_command_buffers);
}

void allocate_post_draw_command_buffers(const u32 i_maxSurfaceTypes)
{
	FLORAL_ASSERT(detail::g_draw_cmdbuff_arena != nullptr);
	detail::g_post_draw_command_buffers.reserve(i_maxSurfaceTypes, detail::g_post_draw_cmdbuff_arena);
}

static void free_all_post_draw_command_buffers()
{
	floral::free_array(detail::g_post_draw_command_buffers);
}

static void free_memory()
{
	floral::free_array(detail::g_framebuffers_pool);
	floral::free_array(detail::g_textures_pool);
	floral::free_array(detail::g_ubs_pool);
	floral::free_array(detail::g_ibs_pool);
	floral::free_array(detail::g_vbs_pool);
	floral::free_array(detail::g_shaders_pool);

	g_persistance_allocator.free(detail::g_resource_arena);
	detail::g_resource_arena = nullptr;

	free_all_post_draw_command_buffers();
	g_persistance_allocator.free(detail::g_post_draw_cmdbuff_arena);
	detail::g_post_draw_cmdbuff_arena = nullptr;

	free_all_draw_command_buffers();
	g_persistance_allocator.free(detail::g_draw_cmdbuff_arena);
	detail::g_draw_cmdbuff_arena = nullptr;
}
//----------------------------------------------

void initialize_render_thread()
{
	g_global_counters.current_render_frame_idx = 0;
	g_global_counters.current_submit_frame_idx = 0;
	g_global_counters.current_frame_idx.store(0, std::memory_order_relaxed);
	g_debug_global_counters.submitted_frames = 0;
	g_debug_global_counters.rendered_frames = 0;

	g_gpu_capacities.ub_max_size = 0;
	g_gpu_capacities.ub_desired_offset = 0;

	// render
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_render_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_render_allocator_size_mb));
		detail::g_frame_draw_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_draw_allocator_size_mb));
		detail::g_render_command_buffer[i].reserve(MAX_RENDER_COMMANDS_IN_BUFFER, &g_persistance_allocator);
	}
	// shading
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_shader_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_shader_allocator_size_mb));
		detail::g_shading_command_buffer[i].reserve(MAX_SHADING_COMMANDS_IN_BUFFER, &g_persistance_allocator);
	}
	// buffers
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_buffers_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_buffers_allocator_size_mb));
		detail::g_buffers_command_buffer[i].reserve(MAX_BUFFERS_COMMANDS_IN_BUFFER, &g_persistance_allocator);
	}
	// textures
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		detail::g_frame_textures_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_textures_allocator_size_mb));
		detail::g_textures_command_buffer[i].reserve(MAX_TEXTURES_COMMANDS_IN_BUFFER, &g_persistance_allocator);
	}
	// ---

	detail::g_composing_cmdbuff = 0;
	detail::g_scene_presented = true;
	detail::g_context_dirty = true;

	s_is_initialized.store(false, std::memory_order_relaxed);
	{
		floral::lock_guard guard(s_resumed_mtx);
		s_resumed = false;
	}

	g_render_thread.entry_point = &insigne::render_thread_func;
	g_render_thread.ptr_data = nullptr;
	g_render_thread.start();
}

void clean_up_render_thread()
{
	// textures
	for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
	{
		floral::free_array(detail::g_textures_command_buffer[i]);
		g_persistance_allocator.free(detail::g_frame_textures_allocator[i]);
		detail::g_frame_textures_allocator[i] = nullptr;
	}
	// buffers
	for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
	{
		floral::free_array(detail::g_buffers_command_buffer[i]);
		g_persistance_allocator.free(detail::g_frame_buffers_allocator[i]);
		detail::g_frame_buffers_allocator[i] = nullptr;
	}
	// shading
	for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
	{
		floral::free_array(detail::g_shading_command_buffer[i]);
		g_persistance_allocator.free(detail::g_frame_shader_allocator[i]);
		detail::g_frame_shader_allocator[i] = nullptr;
	}
	// render
	for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
	{
		floral::free_array(detail::g_render_command_buffer[i]);
		g_persistance_allocator.free(detail::g_frame_draw_allocator[i]);
		detail::g_frame_draw_allocator[i] = nullptr;
		g_persistance_allocator.free(detail::g_frame_render_allocator[i]);
		detail::g_frame_render_allocator[i] = nullptr;
	}
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

void clean_up_and_stop_render_thread()
{
	s_stopped.store(true, std::memory_order_release);
	while (s_is_initialized.load(std::memory_order_acquire))
	{
	}

	// command buffers
	detail::g_waiting_cmdbuffs.clear();

	// memory
	clean_up_render_thread();
	free_memory();
}

void wait_finish_dispatching()
{
	while (!detail::g_waiting_cmdbuffs.is_empty())
	{
	}
}

void unregister_all_surface_types()
{

}

void wait_for_initialization()
{
	while (!s_is_initialized.load(std::memory_order_acquire))
	{
	}
}

}
