#include <iostream>
#include <exception>
#include <chrono>
#include <random>
#include <numbers>
#include <list>

#include <cstdlib>

#include <my-lib/math.h>
#include <my-lib/macros.h>

#include "graphics.h"
#include "debug.h"

// -------------------------------------------

namespace App {

// -------------------------------------------

using Clock = std::chrono::steady_clock;
using ClockDuration = Clock::duration;
using ClockTime = Clock::time_point;

using fp_t = Graphics::fp_t;

using namespace Graphics;

// -------------------------------------------

Renderer *renderer = nullptr;

static bool alive = true;

static constexpr Color config_background_color = {
	.r = 0.0f,
	.g = 0.0f,
	.b = 0.0f,
	.a = 1.0f
};

static std::mt19937_64 rgenerator;

// -------------------------------------------

namespace Config {
	inline constexpr fp_t target_fps = 30.0;
	inline constexpr fp_t min_fps = 20.0; // if fps gets lower than min_fps, we slow down the simulation
	inline constexpr fp_t target_dt = 1.0 / target_fps;
	inline constexpr fp_t max_dt = 1.0 / min_fps;
	inline constexpr fp_t sleep_threshold = target_dt * 0.9;
	inline constexpr bool sleep_to_save_cpu = true;
	inline constexpr bool busy_wait_to_ensure_fps = true;
}

// -------------------------------------------

constexpr ClockDuration fp_to_ClockDuration (const fp_t t)
{
	return std::chrono::duration_cast<ClockDuration>(std::chrono::duration<fp_t>(t));
}

constexpr fp_t ClockDuration_to_fp (const ClockDuration& d)
{
	return std::chrono::duration_cast<std::chrono::duration<fp_t>>(d).count();
}

// -------------------------------------------

class Object
{
protected:
	OO_ENCAPSULATE_OBJ(Point, pos)
public:
	virtual void render (const fp_t dt) = 0;
};

class ObjCube : public Object
{
protected:
	OO_ENCAPSULATE_OBJ(Cube3d, cube)
public:
	void render (const fp_t dt) override final
	{
		renderer->draw_cube3d(this->cube, this->pos);

		constexpr fp_t angular_velocity = Mylib::Math::degrees_to_radians(fp(360)) / fp(2);

		cube.set_rotation_vector(Vector { 1, 1, 0 });
		cube.set_rotation_angle(cube.get_rotation_angle() + angular_velocity * dt);

		dprintln("cube rotation angle=", Mylib::Math::radians_to_degrees(cube.get_rotation_angle()));
	}
};

// -------------------------------------------

std::list<Object*> objects;

// -------------------------------------------

static void setup_random ()
{
	std::random_device rd;
	rgenerator.seed(rd());
}

static Color random_color ()
{
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return { .r = dist(rgenerator), .g = dist(rgenerator), .b = dist(rgenerator), .a = 1.0f };
}

// -------------------------------------------

static void init_objs ()
{
	renderer->set_background_color( { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f } );

	ObjCube& obj_cube = *static_cast<ObjCube*>(objects.emplace_back(new ObjCube));
	obj_cube.set_pos(Point(0, 0, -3));
	Cube3d& cube = obj_cube.get_ref_cube();
	cube.set_w(fp(0.5));
	cube.set_vertex_color(Cube3d::LeftBottomFront,    { .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::RightBottomFront,   { .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::LeftTopFront,       { .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::RightTopFront,      { .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::LeftBottomBack,     { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::RightBottomBack,    { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::LeftTopBack,        { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f });
	cube.set_vertex_color(Cube3d::RightTopBack,       { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f });

#if 1
	for (auto& c : cube.get_colors_ref())
		c = random_color();
#endif
}

// -------------------------------------------

static void render_objs (const fp_t dt)
{
	for (auto *obj : objects)
		obj->render(dt);
}

// -------------------------------------------

static void process_keydown (SDL_KeyboardEvent& event)
{
}

// -------------------------------------------

void process_events ()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				alive = false;
			break;
			
			case SDL_KEYDOWN:
				process_keydown(event.key);
			break;
		}
	}
}

// -------------------------------------------

static void main_loop ()
{
	const Uint8 *keys;
	fp_t real_dt, virtual_dt, required_dt, sleep_dt, busy_wait_dt, fps;

	keys = SDL_GetKeyboardState(nullptr);

	setup_random();
	init_objs();

	real_dt = 0;
	virtual_dt = 0;
	required_dt = 0;
	sleep_dt = 0;
	busy_wait_dt = 0;
	fps = 0;

	while (alive) {
		const ClockTime tbegin = Clock::now();
		ClockTime tend;
		ClockDuration elapsed;

		renderer->wait_next_frame();

		virtual_dt = (real_dt > Config::max_dt) ? Config::max_dt : real_dt;

	#if 1
		dprintln("----------------------------------------------");
		dprintln("start new frame render target_dt=", Config::target_dt,
			" required_dt=", required_dt,
			" real_dt=", real_dt,
			" sleep_dt=", sleep_dt,
			" busy_wait_dt=", busy_wait_dt,
			" virtual_dt=", virtual_dt,
			" max_dt=", Config::max_dt,
			" target_dt=", Config::target_dt,
			" fps=", fps
			);
	#endif

		process_events();

		renderer->setup_projection_matrix({});
		render_objs(virtual_dt);
		renderer->render();

		const ClockTime trequired = Clock::now();
		elapsed = trequired - tbegin;
		required_dt = ClockDuration_to_fp(elapsed);

		if constexpr (Config::sleep_to_save_cpu) {
			if (required_dt < Config::sleep_threshold) {
				sleep_dt = Config::sleep_threshold - required_dt; // target sleep time
				const uint32_t delay = static_cast<uint32_t>(sleep_dt * 1000.0f);
				//dprintln( "sleeping for " << delay << "ms..." )
				SDL_Delay(delay);
			}
		}
		
		const ClockTime tbefore_busy_wait = Clock::now();
		elapsed = tbefore_busy_wait - trequired;
		sleep_dt = ClockDuration_to_fp(elapsed); // check exactly time sleeping

		do {
			tend = Clock::now();
			elapsed = tend - tbegin;
			real_dt = ClockDuration_to_fp(elapsed);

			if constexpr (!Config::busy_wait_to_ensure_fps)
				break;
		} while (real_dt < Config::target_dt);

		elapsed = tend - tbefore_busy_wait;
		busy_wait_dt = ClockDuration_to_fp(elapsed);

		fps = 1.0 / real_dt;
	}
}

// -------------------------------------------

void main (const int argc, char **argv)
{
	renderer = Graphics::init(Renderer::Type::Opengl, 800, 800, false);

	main_loop();

	Graphics::quit(renderer);
}

// -------------------------------------------

} // namespace App

// -------------------------------------------

int main (int argc, char **argv)
{
	try {
		App::main(argc, argv);
	}
	catch (const std::exception& e) {
		std::cout << "Exception happenned!" << std::endl << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "Unknown exception happenned!" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
