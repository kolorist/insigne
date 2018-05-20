namespace insigne {

	template <typename>
	struct draw_command_buffer_t;

	namespace detail {
		// rendering
		template <int t_n, typename t_surface_list>
		struct internal_surface_iterator {
			static void setup_states_and_render()
			{
				typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
				surface_type_t::setup_states();
				surface_type_t::render();
				internal_surface_iterator<t_n - 1, t_surface_list>::setup_states_and_render();
			}
		};

		template <typename t_surface_list>
		struct internal_surface_iterator<0, t_surface_list> {
			static void setup_states_and_render()
			{
				typedef tl_type_at_t<0, t_surface_list> surface_type_t;
				surface_type_t::setup_states();
				surface_type_t::render();
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

			static void clear_buffer(size idx)
			{
				typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
				draw_command_buffer_t<surface_type_t>::command_buffer[idx].empty();
				internal_buffer_iterator<t_n - 1, t_surface_list>::clear_buffer(idx);
			}
		};

		template <typename t_surface_list>
		struct internal_buffer_iterator<0, t_surface_list> {
			static void init_buffer(linear_allocator_t *i_allocator)
			{
				typedef tl_type_at_t<0, t_surface_list> surface_type_t;
				surface_type_t::init_buffer(i_allocator);
			}

			static void clear_buffer(size idx)
			{
				typedef tl_type_at_t<0, t_surface_list> surface_type_t;
				draw_command_buffer_t<surface_type_t>::command_buffer[idx].empty();
			}
		};

		template <typename t_surface_list>
		void internal_init_buffer(linear_allocator_t *i_allocator)
		{
			internal_buffer_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::init_buffer(i_allocator);
		}

		template <typename t_surface_list>
		void internal_clear_buffer(size idx)
		{
			internal_buffer_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::clear_buffer(idx);
		}

	}

}
