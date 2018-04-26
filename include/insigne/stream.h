#pragma once

#include <floral.h>

#include "buffers.h"

namespace insigne {
	void										initialize_stream_thread();

	void										push_command(geometry_stream_command& i_cmd);
}
