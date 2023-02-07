/* -*- C++ -*-
*
*  gles_renderer.h
*
*  Copyright (C) 2022 jh10001 <jh10001@live.cn>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#pragma once
#include <SDL.h>
#include <SDL_opengles2.h>

class GlesRenderer {
    SDL_Window *window;
	SDL_Texture *texture;
	SDL_GLContext context;
	GLuint vert_shader;
	GLuint frag_shader;
	GLuint post_program;
	GLuint vertex_buffer;
	GLfloat vertex_data[8];
	GLfloat cas_con[4*2];
	GLint const_buffer_location[3];
	int output_size[2];
	bool _pause = false;
#if !(defined(IOS) || defined(ANDROID))
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "gles2funcs.h"
#undef SDL_PROC
#endif
	GLuint createShader(GLenum shader_type, const GLchar* shader_src);
	void initVertexData();
public:
	GlesRenderer(SDL_Window *window, SDL_Texture *texture, const float input_size[2], const float output_size[2], float sharpness);
	~GlesRenderer();

	void copy(const int window_x, const int window_y);
	void setConstBuffer(const float input_size[2], const float output_size[2], float sharpness);
	void pause();
	void resume();
};
