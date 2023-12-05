#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>
#include <array>

#include <cstdlib>
#include <cmath>

#include <my-lib/std.h>

#include "graphics.h"

#ifdef SUPPORT_OPENGL
	#include "opengl/opengl.h"
#endif
#ifdef SUPPORT_VULKAN
	#error "Vulkan is not supported yet!"
#endif

namespace Graphics {

// ---------------------------------------------------

const char* Renderer::get_type_str (const Type value)
{
	static constexpr auto strs = std::to_array<const char*>({
		"Opengl",
		"Vulkan"
	});

	mylib_assert_exception_msg(std::to_underlying(value) < strs.size(), "invalid enum class value ", std::to_underlying(value))

	return strs[ std::to_underlying(value) ];
}

Renderer* init (const Renderer::Type renderer_type, const uint32_t screen_width_px, const uint32_t screen_height_px, const bool fullscreen)
{
	Renderer *r;

	switch (renderer_type) {
		case Renderer::Type::Opengl:
			r = new Opengl::Renderer(screen_width_px, screen_height_px, fullscreen);
		break;

		default:
			throw std::runtime_error("Bad Video Driver!");
	}

	return r;
}

void quit (Renderer *renderer)
{

}

// ---------------------------------------------------

} // namespace Graphics