#include "insigne/counters.h"

namespace insigne {

global_counters									g_global_counters;
debug_global_counters							g_debug_global_counters;
debug_frame_counters							g_debug_frame_counters[debug_frames_count];
hardware_counters*								g_hardware_counters = nullptr;

const u64 get_current_frame_idx()
{
	return g_global_counters.current_frame_idx.load();
}

const u64 get_valid_debug_info_range_begin()
{
	u64 rangeBegin = g_global_counters.current_read_range_begin.load(std::memory_order_acquire);
	return (rangeBegin % debug_frames_count);
}

const u64 get_valid_debug_info_range_end()
{
	u64 rangeEnd = g_global_counters.current_read_range_end.load(std::memory_order_acquire);
	return (rangeEnd % debug_frames_count);
}

}
