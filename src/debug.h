#ifndef __CUBE3D_SDL_OPENGL_DEBUG_HEADER_H__
#define __CUBE3D_SDL_OPENGL_DEBUG_HEADER_H__

// ---------------------------------------------------

#define DEBUG

// ---------------------------------------------------

#if defined(DEBUG) && !defined(__ANDROID__)
	#include <iostream>
	#include <iomanip>
#endif

#include <my-lib/std.h>

#include <SDL.h>

namespace App
{

// ---------------------------------------------------

#ifdef DEBUG
	#ifdef __ANDROID__
		template <typename... Types>
		void dprint (Types&&... vars)
		{
			const std::string str = Mylib::build_str_from_stream("CUBE3D: ", vars...);
			SDL_Log("%s", str.data());
		}

		template <typename... Types>
		void dprintln (Types&&... vars)
		{
			dprint(vars..., '\n');
		}

		inline constexpr void debug_config_stream ()
		{
		}
	#else
		template <typename... Types>
		void dprint (Types&&... vars)
		{
			Mylib::print_stream(std::cout, vars...);
		}

		template <typename... Types>
		void dprintln (Types&&... vars)
		{
			dprint(vars..., '\n');
		}

		inline void debug_config_stream ()
		{
			std::cout << std::setprecision(4);
			std::cout << std::fixed;
		}
	#endif
#else
	#define dprint(...)
	#define dprintln(...)

	inline constexpr void debug_config_stream ()
	{
	}
#endif

// ---------------------------------------------------

} // end namespace

#endif