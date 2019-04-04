#pragma once

#include <floral.h>

#include "generated_code/proxy.h"

#include "context.h"
#include <context.h>

namespace insigne {

void											initialize_driver();

void											create_main_context();
void											refresh_context();
void											create_shared_context();

void											swap_buffers();

//----------------------------------------------

gl_debug_info&									get_driver_info();
void											request_refresh_context();

}
