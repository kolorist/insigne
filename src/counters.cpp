#include "insigne/counters.h"

namespace insigne {

global_counters									g_global_counters;
debug_global_counters							g_debug_global_counters;
debug_frame_counters							g_debug_frame_counters;

const u64 get_current_frame_idx() {
	return g_global_counters.current_frame_idx.load();
}

}
