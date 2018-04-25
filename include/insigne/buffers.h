#pragma once

#include <stdaliases.h>
#include <containers/array.h>

namespace insigne {

	enum class command {
		// render
		draw_geom_index = 0,
		draw_geom_noindex,
		// geometry stream
		upload_geom,
		// texture stream
		upload_texture,
		// shader stream
		create_shader
	};

	struct gpu_command {
		command									opcode;
	};

	struct render_command : gpu_command {
		
	};

	struct geometry_stream_command : gpu_command {
	};

	struct texture_stream_command : gpu_command {
	};

	struct shader_stream_command : gpu_command {
	};

}
