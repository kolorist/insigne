#pragma once

namespace insigne {
namespace detail {

//----------------------------------------------
struct shader_desc_t {
	floral::path								vs_path, fs_path;
	const_cstr									vs, fs;
};

struct material_desc_t {
};

//----------------------------------------------
struct vbdesc_t {
};

struct ibdesc_t {
};

struct ubdesc_t {
};

}
}
