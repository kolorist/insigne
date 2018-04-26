#pragma once

#include <floral.h>

#include "generated_code/proxy.h"

#include "buffers.h"

namespace insigne {

	void										initialize_render_thread();
	void										wait_for_initialization();

	void										begin_frame();
	void										end_frame();
	void										dispatch_frame();

	void										push_command(render_command& i_cmd);

}
