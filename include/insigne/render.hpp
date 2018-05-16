namespace insigne {

	// -----------------------------------------
	// rendering
	template <int t_n, typename t_surface_list>
	struct internal_surface_iterator {
		static void setup_states_and_render()
		{
			typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
			surface_type_t::setup_states_and_render();
			internal_surface_iterator<t_n - 1, t_surface_list>::setup_states_and_render();
		}
	};

	template <typename t_surface_list>
	struct internal_surface_iterator<0, t_surface_list> {
		static void setup_states_and_render()
		{
			typedef tl_type_at_t<0, t_surface_list> surface_type_t;
			surface_type_t::setup_states_and_render();
		}
	};
	
	template <typename t_surface_list>
	void internal_render_surfaces()
	{
		internal_surface_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::setup_states_and_render();
	}

	// init gpu command buffer
	template <int t_n, typename t_surface_list>
	struct internal_buffer_iterator {
		static void init_buffer(linear_allocator_t *i_allocator)
		{
			typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
			surface_type_t::init_buffer(i_allocator);
			internal_buffer_iterator<t_n - 1, t_surface_list>::init_buffer(i_allocator);
		}
	};

	template <typename t_surface_list>
	struct internal_buffer_iterator<0, t_surface_list> {
		static void init_buffer(linear_allocator_t *i_allocator)
		{
			typedef tl_type_at_t<0, t_surface_list> surface_type_t;
			surface_type_t::init_buffer(i_allocator);
		}
	};

	template <typename t_surface_list>
	void internal_init_buffer(linear_allocator_t *i_allocator)
	{
		internal_buffer_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::init_buffer(i_allocator);
	}
	
	// -----------------------------------------
	template <typename t_surface_list>
	void render_thread_func(voidptr i_data)
	{
		// buffer setup phase
		// render phase
		internal_render_surfaces<t_surface_list>();
	}

	template <typename t_surface_list>
	void initialize_render_thread()
	{
		FLORAL_ASSERT_MSG(sizeof(init_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(load_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_state_toggle_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");

		internal_init_buffer<t_surface_list>(&g_persistance_allocator);

		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			s_gpu_frame_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(SIZE_MB(8));

		s_materials.init(32, &g_persistance_allocator);

		s_front_cmdbuff = 0;
		s_back_cmdbuff = 2;
		s_render_state_changelog = 0;

		g_render_thread.entry_point = &insigne::render_thread_func<t_surface_list>;
		g_render_thread.ptr_data = nullptr;
		g_render_thread.start();
	}
}
