#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>

#include <cstdlib>
#include <cmath>

#include <my-lib/math.h>

#include "../debug.h"
#include "opengl.h"

// ---------------------------------------------------

using App::dprint;
using App::dprintln;

#define DEBUG_SHOW_CENTER_LINE

// ---------------------------------------------------

namespace Graphics
{
namespace Opengl
{

// ---------------------------------------------------

Shader::Shader (const GLenum shader_type_, const char *fname_)
: shader_type(shader_type_),
  fname(fname_)
{
	this->shader_id = glCreateShader(this->shader_type);
}

void Shader::compile ()
{
	std::ifstream t(this->fname);
	std::stringstream str_stream;
	str_stream << t.rdbuf();
	std::string buffer = str_stream.str();

	dprintln("loaded shader (", this->fname, ")");
	//dprint( buffer )
	
	const char *c_str = buffer.c_str();
	glShaderSource(this->shader_id, 1, ( const GLchar ** )&c_str, nullptr);
	glCompileShader(this->shader_id);

	GLint status;
	glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &logSize);

		char *berror = new char [logSize];

		glGetShaderInfoLog(this->shader_id, logSize, nullptr, berror);

		mylib_throw_exception_msg(this->fname, " shader compilation failed", '\n', berror);
	}
}

Program::Program ()
{
	this->vs = nullptr;
	this->fs = nullptr;
	this->program_id = glCreateProgram();
}

void Program::attach_shaders ()
{
	glAttachShader(this->program_id, this->vs->shader_id);
	glAttachShader(this->program_id, this->fs->shader_id);
}

void Program::link_program ()
{
	glLinkProgram(this->program_id);
}

void Program::use_program ()
{
	glUseProgram(this->program_id);
}

ProgramTriangle::ProgramTriangle ()
	: Program ()
{
	static_assert(sizeof(Vector) == sizeof(fp_t) * 3);
	static_assert(sizeof(Vector) == sizeof(Point));
	static_assert(sizeof(Color) == sizeof(float) * 4);
	static_assert(sizeof(Vertex) == (sizeof(Point) + sizeof(Vector) + sizeof(Color)));

	this->vs = new Shader(GL_VERTEX_SHADER, "shaders/triangles.vert");
	this->vs->compile();

	this->fs = new Shader(GL_FRAGMENT_SHADER, "shaders/triangles.frag");
	this->fs->compile();

	this->attach_shaders();

	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Position), "i_position");
	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Offset), "i_offset");
	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Color), "i_color");

	this->link_program();

	glGenVertexArrays(1, &(this->vao));
	glGenBuffers(1, &(this->vbo));
}

void ProgramTriangle::bind_vertex_array ()
{
	glBindVertexArray(this->vao);
}

void ProgramTriangle::bind_vertex_buffer ()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
}

void ProgramTriangle::setup_vertex_array ()
{
	uint32_t pos, length;

	glEnableVertexAttribArray( std::to_underlying(Attrib::Position) );
	glEnableVertexAttribArray( std::to_underlying(Attrib::Offset) );
	glEnableVertexAttribArray( std::to_underlying(Attrib::Color) );

	pos = 0;
	length = 3;
	glVertexAttribPointer( std::to_underlying(Attrib::Position), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 3;
	glVertexAttribPointer( std::to_underlying(Attrib::Offset), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 4;
	glVertexAttribPointer( std::to_underlying(Attrib::Color), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
}

void ProgramTriangle::upload_vertex_buffer ()
{
	uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * n, this->triangle_buffer.get_vertex_buffer(), GL_DYNAMIC_DRAW);
}

void ProgramTriangle::upload_projection_matrix (const Matrix4& m)
{
	glUniformMatrix4fv( glGetUniformLocation(this->program_id, "u_projection_matrix"), 1, GL_TRUE, m.get_raw() );
	//dprintln( "projection matrix sent to GPU" )
}

void ProgramTriangle::draw ()
{
	uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glDrawArrays(GL_TRIANGLES, 0, n);
}

void ProgramTriangle::debug ()
{
	uint32_t n = this->triangle_buffer.get_vertex_buffer_used();

	for (uint32_t i=0; i<n; i++) {
		Vertex *v = this->triangle_buffer.get_vertex(i);

		if ((i % 3) == 0)
			dprintln();

		dprintln("vertex[", i,
			"] x=", v->local_pos.x,
			" y=", v->local_pos.y,
			" z=", v->local_pos.z,
			" offset_x=", v->offset.x,
			" offset_y=", v->offset.y,
			" offset_z=", v->offset.z,
			" r=", v->color.r,
			" g=", v->color.g,
			" b=", v->color.b,
			" a=", v->color.a
		);
	}
}

Renderer::Renderer (const uint32_t window_width_px_, const uint32_t window_height_px_, const bool fullscreen_)
	: Graphics::Renderer (window_width_px_, window_height_px_, fullscreen_)
{
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	this->sdl_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->window_width_px, this->window_height_px, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	this->sdl_gl_context = SDL_GL_CreateContext(this->sdl_window);

	GLenum err = glewInit();

	mylib_assert_exception_msg(err == GLEW_OK, "Error: ", glewGetErrorString(err))

	dprintln("Status: Using GLEW ", glewGetString(GLEW_VERSION));

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	glClearColor(this->background_color.r, this->background_color.g, this->background_color.b, 1.0);
	glViewport(0, 0, this->window_width_px, this->window_height_px);

	this->load_opengl_programs();

	dprintln("loaded opengl stuff");

	this->wait_next_frame();
}

void Renderer::load_opengl_programs ()
{
	this->program_triangle = new ProgramTriangle;

	dprintln("loaded opengl triangle program");

	this->program_triangle->use_program();
	
	this->program_triangle->bind_vertex_array();
	this->program_triangle->bind_vertex_buffer();

	this->program_triangle->setup_vertex_array();

	dprintln("generated and binded opengl world vertex array/buffer");
}

Renderer::~Renderer ()
{
	delete this->program_triangle;

	SDL_GL_DeleteContext(this->sdl_gl_context);
	SDL_DestroyWindow(this->sdl_window);
}

void Renderer::wait_next_frame ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->program_triangle->clear();
}

void Renderer::draw_cube3d (const Cube3d& cube, const Vector& offset)
{
	const Vector local_pos = cube.get_value_delta();
	//const Vector world_pos = Vector(4.0f, 4.0f);

	using PositionIndex = Cube3d::PositionIndex;
	using enum PositionIndex;
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::array<Point, 8> points;

	points[LeftTopFront] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[LeftBottomFront] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[RightTopFront] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[RightBottomFront] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[LeftTopBack] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[LeftBottomBack] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[RightTopBack] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[RightBottomBack] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	if (cube.get_rotation_angle() != fp(0)) {
		for (auto& p : points)
			p = Mylib::Math::rotate_around_vector(p, cube.get_ref_rotation_vector(), cube.get_rotation_angle());
	}
	
	constexpr uint32_t n_triangles = 12; // 6 faces * 2 triangles per face
	constexpr uint32_t n_vertices = n_triangles * 3;

	ProgramTriangle::Vertex *vertices = this->program_triangle->alloc_vertices(n_vertices);
	uint32_t i = 0;

	auto mount = [&i, vertices, &points, &cube, &offset] (const PositionIndex p) -> void {
		vertices[i].local_pos = points[p];
		vertices[i].offset = offset;
		vertices[i].color = cube.get_vertex_color(p);
		i++;
	};

	auto mount_triangle = [&mount] (const PositionIndex p1, const PositionIndex p2, const PositionIndex p3) -> void {
		mount(p1);
		mount(p2);
		mount(p3);
	};

	// bottom
	mount_triangle(LeftBottomFront, RightBottomFront, LeftBottomBack);
	mount_triangle(RightBottomBack, RightBottomFront, LeftBottomBack);

	// top
	mount_triangle(LeftTopFront, RightTopFront, LeftTopBack);
	mount_triangle(RightTopBack, RightTopFront, LeftTopBack);

	// front
	mount_triangle(LeftTopFront, LeftBottomFront, RightTopFront);
	mount_triangle(RightBottomFront, LeftBottomFront, RightTopFront);

	// back
	mount_triangle(LeftTopBack, LeftBottomBack, RightTopBack);
	mount_triangle(RightBottomBack, LeftBottomBack, RightTopBack);

	// left
	mount_triangle(LeftTopFront, LeftBottomFront, LeftTopBack);
	mount_triangle(LeftBottomBack, LeftBottomFront, LeftTopBack);

	// right
	mount_triangle(RightTopFront, RightBottomFront, RightTopBack);
	mount_triangle(RightBottomBack, RightBottomFront, RightTopBack);

	mylib_assert_exception(i == n_vertices)
}

void Renderer::setup_projection_matrix (const RenderArgs& args)
{
	this->projection_matrix = Mylib::Math::gen_perspective_matrix<fp_t>(
		Mylib::Math::degrees_to_radians(fp(45)),
		static_cast<fp_t>(this->window_width_px),
		static_cast<fp_t>(this->window_height_px),
		fp(0.1),
		fp(100),
		fp(-1)
	);

	this->projection_matrix.transpose();

	//this->projection_matrix = Mylib::Math::gen_identity_matrix<fp_t, 4>();

#if 1
	dprintln("projection matrix:");
	dprintln(this->projection_matrix);
	dprintln();
#endif
}

void Renderer::render ()
{
	//this->program_triangle->debug();
	this->program_triangle->upload_projection_matrix(this->projection_matrix);
	this->program_triangle->upload_vertex_buffer();
	this->program_triangle->draw();
	SDL_GL_SwapWindow(this->sdl_window);
}

// ---------------------------------------------------

} // namespace Opengl
} // namespace Graphics