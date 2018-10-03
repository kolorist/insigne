#include "context.h"
#include "driver.h"
#include "counters.h"
#include "internal_states.h"
#include "detail/rt_shading.h"
#include "detail/rt_buffers.h"
#include "detail/rt_render.h"

namespace insigne {

// -----------------------------------------
template <typename t_surface_list>
void initialize_renderer()
{
	insigne::detail::initialize_shading_module();
	insigne::detail::initialize_buffers_module();
	insigne::detail::initialize_render_module<t_surface_list>();

	// do we really need this?
	GLuint vao;
	pxGenVertexArrays(1, &vao);
	pxBindVertexArray(vao);
}

// -----------------------------------------
/* entry point of rendering thread
 */
template <typename t_surface_list>
void render_thread_func(voidptr i_data)
{
	// profiler init
	lotus::init_capture_for_this_thread(1, "render_thread");
	create_main_context();
	initialize_renderer<t_surface_list>();
	detail::g_init_condvar.notify_one();

	while (true) {
		{
			while (detail::g_front_cmdbuff == detail::g_back_cmdbuff)
				detail::g_cmdbuffer_condvar.wait(detail::g_cmdbuffer_mtx);
		}

		g_global_counters.current_render_frame_idx++;

		detail::process_shading_command_buffer();
		detail::process_buffers_command_buffer();
		//detail::process_textures_command_buffer();
		bool swapThisRenderPass = detail::process_render_command_buffer();
		detail::process_draw_command_buffer<t_surface_list>();

		detail::g_front_cmdbuff = (detail::g_front_cmdbuff + 1) % BUFFERS_COUNT;
		if (swapThisRenderPass) {
			PROFILE_SCOPE(SwapBuffers);
			swap_buffers();
			detail::s_waiting_for_swap = false;
			g_debug_global_counters.rendered_frames++;
		}
	}
}

// ---------------------------------------------

template <typename t_surface_list>
void initialize_render_thread()
{
	FLORAL_ASSERT_MSG(sizeof(init_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
	FLORAL_ASSERT_MSG(sizeof(render_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
	FLORAL_ASSERT_MSG(sizeof(load_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
	FLORAL_ASSERT_MSG(sizeof(render_state_toggle_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");

	g_global_counters.current_render_frame_idx = 0;
	g_global_counters.current_submit_frame_idx = 0;
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

	detail::g_front_cmdbuff = 0;
	detail::g_back_cmdbuff = BUFFERS_COUNT - 1;
	detail::s_waiting_for_swap = true;

	g_render_thread.entry_point = &insigne::render_thread_func<t_surface_list>;
	g_render_thread.ptr_data = nullptr;
	g_render_thread.start();
}

}
