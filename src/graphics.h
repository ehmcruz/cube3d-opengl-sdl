#ifndef __CUBE3D_SDL_GRAPHICS_HEADER_H__
#define __CUBE3D_SDL_GRAPHICS_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>

#include <cstring>

#include <string>
#include <algorithm>
#include <array>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/any.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-vector.h>

// ---------------------------------------------------

namespace Graphics
{

using fp_t = float;
using Vector3 = Mylib::Math::Vector3f;
using Matrix4 = Mylib::Math::Matrix4f;
using Vector4 = Mylib::Math::Vector4f;
using Vector = Vector3;
using Point = Vector;

// ---------------------------------------------------

struct Color {
	float r;
	float g;
	float b;
	float a; // alpha
};

// ---------------------------------------------------

class Shape
{
public:
	enum class Type {
		Cube3d,
	};
protected:
	OO_ENCAPSULATE_SCALAR_READONLY(Type, type)

	// distance from the center of the shape to the center of the object
	OO_ENCAPSULATE_OBJ(Vector, delta)

public:
	Shape (const Type type_) noexcept
		: type(type_), delta(Vector::zero())
	{
	}
};

// ---------------------------------------------------

class Cube3d: public Shape
{
public:
	static consteval uint32_t get_n_vertices ()
	{
		return 8;
	}

protected:
	OO_ENCAPSULATE_SCALAR(fp_t, width)
	OO_ENCAPSULATE_OBJ(std::array<Color, get_n_vertices()>, colors)

public:
	Cube3d (const fp_t width_) noexcept
		: Shape(Type::Cube3d), width(width_)
	{
		//dprint( "circle created r=" << this->radius << std::endl )
	}

	Cube3d ()
		: Cube3d(0)
	{
	}
};

// ---------------------------------------------------

struct RenderArgs {
	Vector world_camera_focus;
};

// ---------------------------------------------------

class Renderer
{
public:
	enum class Type { // any change here will need a change in get_type_str
		Opengl,
		Vulkan,
		Unsupported // must be the last one
	};

	static const char* get_type_str (const Type t);

protected:
	SDL_Window *sdl_window;
	OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_width_px)
	OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_height_px)
	OO_ENCAPSULATE_SCALAR_READONLY(bool, fullscreen)
	OO_ENCAPSULATE_SCALAR_READONLY(float, window_aspect_ratio)

public:
	inline Renderer (const uint32_t window_width_px_, const uint32_t window_height_px_, const bool fullscreen_)
		: window_width_px(window_width_px_), window_height_px(window_height_px_), fullscreen(fullscreen_)
	{
		this->window_aspect_ratio = static_cast<float>(this->window_width_px) / static_cast<float>(this->window_height_px);
	}

	inline float get_inverted_window_aspect_ratio () const
	{
		return 1.0f / this->window_aspect_ratio;
	}

	inline Vector get_normalized_window_size () const
	{
		const float max_value = static_cast<float>( std::max(this->window_width_px, this->window_height_px) );
		return Vector(static_cast<float>(this->window_width_px) / max_value, static_cast<float>(this->window_height_px) / max_value);
	}

	virtual void wait_next_frame () = 0;
	virtual void draw_cube3d (const Cube3d& rect, const Vector& offset) = 0;
	virtual void setup_projection_matrix (const RenderArgs& args) = 0;
	virtual void render () = 0;
};

// ---------------------------------------------------

Renderer* init (const Renderer::Type renderer_type, const uint32_t screen_width_px, const uint32_t screen_height_px, const bool fullscreen);
void quit (Renderer *renderer, const Renderer::Type renderer_type);

// ---------------------------------------------------

} // end namespace Graphics

#endif