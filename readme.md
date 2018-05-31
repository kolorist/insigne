# What does 'insigne' do?
* initialize rendering context (after acquiring drawing surface)
	- rendering thread
	- streamming thread
* rendering api errors checking proxy
* generalize drawing commands: draw calls, resource upload / update, ...
* triangle-level optimization: batching, culling, ...

# How to add a shader param list
- include/insigne/internal_commons.h
	add to material_template_t