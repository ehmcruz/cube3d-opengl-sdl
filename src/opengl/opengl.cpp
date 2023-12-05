#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>

#include <cstdlib>
#include <cmath>

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
	static_assert(sizeof(Vertex) == 32);
	static_assert((sizeof(Vertex) / sizeof(GLfloat)) == 8);

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
	length = 2;
	glVertexAttribPointer( std::to_underlying(Attrib::Position), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 2;
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
			"] x=", v->x,
			" y= ", v->y,
			" offset_x= ", v->offset_x,
			" offset_y= ", v->offset_y,
			" r= ", v->r,
			" g= ", v->g,
			" b= ", v->b,
			" a= ", v->a
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

	glDisable(GL_DEPTH_TEST);

	this->background_color = Graphics::config_background_color;

	glClearColor(this->background_color.r, this->background_color.g, this->background_color.b, 1.0);
	glViewport(0, 0, this->window_width_px, this->window_height_px);

	this->circle_factory_low_def = new CircleFactory(64);
	//this->opengl_circle_factory_high_def = new Opengl::CircleFactory(Config::opengl_high_def_triangles);

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
	delete this->circle_factory_low_def;

	SDL_GL_DeleteContext(this->sdl_gl_context);
	SDL_DestroyWindow(this->sdl_window);
}

void Renderer::wait_next_frame ()
{
	glClear( GL_COLOR_BUFFER_BIT );

	this->program_triangle->clear();
}

void Renderer::draw_cube3d (const Cube3d& rect, const Vector& offset)
{
	const uint32_t n_vertices = 6;
	const Vector local_pos = rect.get_value_delta();
	//const Vector world_pos = Vector(4.0f, 4.0f);
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	ProgramTriangle::Vertex *vertices = this->program_triangle->alloc_vertices(n_vertices);

	// draw first triangle

	// upper left vertex
	vertices[0].x = local_pos.x - rect.get_w()*0.5f;
	vertices[0].y = local_pos.y - rect.get_h()*0.5f;

	// down right vertex
	vertices[1].x = local_pos.x + rect.get_w()*0.5f;
	vertices[1].y = local_pos.y + rect.get_h()*0.5f;

	// down left vertex
	vertices[2].x = local_pos.x - rect.get_w()*0.5f;
	vertices[2].y = local_pos.y + rect.get_h()*0.5f;

	// draw second triangle

	// upper left vertex
	vertices[3].x = local_pos.x - rect.get_w()*0.5f;
	vertices[3].y = local_pos.y - rect.get_h()*0.5f;

	// upper right vertex
	vertices[4].x = local_pos.x + rect.get_w()*0.5f;
	vertices[4].y = local_pos.y - rect.get_h()*0.5f;

	// down right vertex
	vertices[5].x = local_pos.x + rect.get_w()*0.5f;
	vertices[5].y = local_pos.y + rect.get_h()*0.5f;

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].offset_x = offset.x;
		vertices[i].offset_y = offset.y;
		vertices[i].r = color.r;
		vertices[i].g = color.g;
		vertices[i].b = color.b;
		vertices[i].a = color.a;
	}
}

void Renderer::setup_projection_matrix (const RenderArgs& args)
{

}

void Renderer::render ()
{
	this->program_triangle->upload_projection_matrix(this->projection_matrix);
	this->program_triangle->upload_vertex_buffer();
	this->program_triangle->draw();
	SDL_GL_SwapWindow(this->sdl_window);
}
