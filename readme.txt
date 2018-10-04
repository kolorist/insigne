# What does 'insigne' do?
* initialize rendering context (after acquiring drawing surface)
	- rendering thread
	- streamming thread
* rendering api errors checking proxy
* generalize drawing commands: draw calls, resource upload / update, ...
* triangle-level optimization: batching, culling, ...

# Structure
* "ut_" prefix
	- Those files with this prefix consists of "user thread" functions that user can use to
	dispatch work to the "render thread"
* "rt_" prefix
	- Internal use for "render thread"
* key locations:
	> render thread initialize					system.hpp
	> modules initialize						system.hpp
	> render thread entry point					system.hpp

	> shading command buffer process			detail/rt_shading.cpp
	> buffers command buffer process			detail/rt_buffers.cpp
	> textures command buffer process			<tbd>
	> render command buffer process				detail/rt_render.cpp
	> draw command buffer process				detail/rt_render.hpp
		* This is the most complex command buffer process routine
