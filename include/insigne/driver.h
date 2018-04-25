#pragma once

#include <stdaliases.h>

#include "generated_code/proxy.h"

#include "context.h"

namespace insigne {
	void										initialize_driver(HWND i_hwnd);

	void										create_main_context();
	void										create_shared_context();

	void										swap_buffers();
}
