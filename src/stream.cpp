#include "insigne/stream.h"

#include <floral.h>

#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/generated_code/proxy.h"

namespace insigne {
	// -----------------------------------------
	typedef floral::fixed_array<geometry_stream_command, linear_allocator_t>	geometry_stream_command_buffer_t;
	typedef floral::fixed_array<texture_stream_command, linear_allocator_t>		texture_stream_command_buffer_t;
	
	static geometry_stream_command_buffer_t		s_geom_stream_command_buffer;
	static texture_stream_command_buffer_t		s_tex_stream_command_buffer;
	// -----------------------------------------
	static floral::condition_variable			s_cmdbuffer_condvar;
	static floral::mutex						s_cmdbuffer_mtx;

	void stream_thread_func(voidptr i_data)
	{
		while(true)
		{
			s_cmdbuffer_condvar.wait(s_cmdbuffer_mtx);
		}
	}

	// -----------------------------------------
	void initialize_stream_thread()
	{
		g_stream_thread.entry_point = &insigne::stream_thread_func;
		g_stream_thread.ptr_data = nullptr;
		g_stream_thread.start();
	}
}
