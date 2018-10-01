namespace insigne {
namespace detail {

inline void initialize_buffers_module()
{
	// TODO: hardcode!!!
	g_vbs_pool.init(128u, &g_persistance_allocator);
	g_ibs_pool.init(128u, &g_persistance_allocator);
	g_ubs_pool.init(64u, &g_persistance_allocator);
}

}
}
