#ifndef __CUBE3D_SDL_GRAPHICS_HEADER_H__
#define __CUBE3D_SDL_GRAPHICS_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>

#include <cstring>
#include <cmath>

#include <string>
#include <algorithm>
#include <array>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/any.h>
#include <my-lib/math.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-geometry.h>

// ---------------------------------------------------

namespace Graphics
{

using fp_t = float;
using Vector2 = Mylib::Math::Vector2f;
using Vector3 = Mylib::Math::Vector3f;
using Vector4 = Mylib::Math::Vector4f;
using Line = Mylib::Math::Line3f;
using Matrix4 = Mylib::Math::Matrix<float, 4, 4>;
using Point2 = Vector2;
using Point3 = Vector3;
using Point4 = Vector4;
using Vector = Vector3;
using Point = Vector;

// ---------------------------------------------------

consteval fp_t fp (const auto v)
{
	return static_cast<fp_t>(v);
}

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

	enum PositionIndex {
		LeftTopFront,
		LeftBottomFront,
		RightTopFront,
		RightBottomFront,
		LeftTopBack,
		LeftBottomBack,
		RightTopBack,
		RightBottomBack
	};

protected:
	OO_ENCAPSULATE_SCALAR(fp_t, w) // width

	OO_ENCAPSULATE_SCALAR_INIT(fp_t, rotation_angle, 0)
	OO_ENCAPSULATE_OBJ(Vector, rotation_vector)

	std::array<Color, 8> colors;

public:
	Cube3d (const fp_t w_) noexcept
		: Shape(Type::Cube3d), w(w_)
	{
		//dprint( "circle created r=" << this->radius << std::endl )
	}

	Cube3d ()
		: Cube3d(0)
	{
	}

	constexpr void set_rotation_angle_bounded (const fp_t angle) noexcept
	{
		this->rotation_angle = std::fmod(angle, Mylib::Math::degrees_to_radians(fp(360)));
	}

	void set_vertex_color (const PositionIndex i, const Color& color) noexcept
	{
		//mylib_assert_exception(i < get_n_vertices())
		this->colors[i] = color;
	}

	const Color& get_vertex_color (const PositionIndex i) const noexcept
	{
		//mylib_assert_exception(i < get_n_vertices())
		return this->colors[i];
	}

	std::array<Color, 8>& get_colors_ref () noexcept
	{
		return this->colors;
	}

	inline fp_t get_h () const noexcept
	{
		return this->w;
	}

	inline fp_t get_d () const noexcept
	{
		return this->w;
	}
};

// ---------------------------------------------------

struct RenderArgs {
	Line world_camera;
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
	OO_ENCAPSULATE_OBJ(Color, background_color)

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

	inline Vector2 get_normalized_window_size () const
	{
		const float max_value = static_cast<float>( std::max(this->window_width_px, this->window_height_px) );
		return Vector2(static_cast<float>(this->window_width_px) / max_value, static_cast<float>(this->window_height_px) / max_value);
	}

	virtual void wait_next_frame () = 0;
	virtual void draw_cube3d (const Cube3d& cube, const Vector& offset) = 0;
	virtual void setup_projection_matrix (const RenderArgs& args) = 0;
	virtual void render () = 0;
};

// ---------------------------------------------------

Renderer* init (const Renderer::Type renderer_type, const uint32_t screen_width_px, const uint32_t screen_height_px, const bool fullscreen);
void quit (Renderer *renderer);

// ---------------------------------------------------

} // end namespace Graphics

#endif