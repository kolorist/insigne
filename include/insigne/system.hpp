namespace insigne {

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

	// generic buffer init
	for (u32 i = 0; i < BUFFERED_FRAMES; i++)
		detail::s_generic_command_buffer[i].init(g_settings.generic_command_buffer_size, &g_persistance_allocator);
	// draw buffer init
	detail::internal_init_buffer<t_surface_list>(&g_persistance_allocator);

	for (u32 i = 0; i < BUFFERED_FRAMES; i++)
		detail::s_gpu_frame_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
				SIZE_MB(g_settings.frame_allocator_size_mb));

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
	// ---

	detail::s_materials.init(32, &g_persistance_allocator);
	g_debug_global_counters.materials_cap = detail::s_materials.get_size();

	detail::s_front_cmdbuff = 0;
	detail::s_back_cmdbuff = BUFFERS_COUNT - 1;
	detail::s_render_state_changelog = 0;
	detail::s_waiting_for_swap = true;

	g_render_thread.entry_point = &insigne::render_thread_func<t_surface_list>;
	g_render_thread.ptr_data = nullptr;
	g_render_thread.start();
}

}
