/* -*- C++ -*-
 *
 *  gles_renderer.cpp
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

#include "gles_renderer.h"
#include "../Utils.h"
#include "shader/post_cas.h"

#if (defined(IOS) || defined(ANDROID))
#include <GLES2/gl2.h>
#endif

#define GLES_CHECK_ERROR(tag) \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            utils::printError("%s: GLES error: (0x%X)\n", tag, err); \
        } \
    } while ( 0 );

const static GLchar *post_vert_src = R"END(#version 300 es
in vec2 a_position;
void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)END";

//static GLuint AU1_AF1(GLfloat a){union{GLfloat f;GLuint u;}bits;bits.f=a;return bits.u;}

static void casSetup(GLfloat con[4*2], float sharpness, const float inputSizeInPixels[2], const float outputSizeInPixels[2]) {
    // Scaling terms.
    con[0] = inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]);
    con[1] = inputSizeInPixels[1] * (1.0f / outputSizeInPixels[1]);
    con[2] = 0.5f * inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]) - 0.5f;
    con[3] = 0.5f * inputSizeInPixels[1] * (1.0f / outputSizeInPixels[1]) - 0.5f;
    // Sharpness value.
    const float sharp = 1.0f / (8.0f - sharpness * 3.0f);
    con[4] = con[5] = sharp;
    con[6] = 8.0f * inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]);
    con[7] = inputSizeInPixels[1];
}

void GlesRenderer::setConstBuffer(const float input_size[2], const float output_size[2], const float sharpness) {
    this->output_size[0] = output_size[0];
    this->output_size[1] = output_size[1];
    casSetup(cas_con, sharpness, input_size, output_size);
}

void GlesRenderer::initVertexData() {
    GLfloat minu = 0, maxu = 1, minv = 0, maxv = 1;

    vertex_data[0] = minu;
    vertex_data[1] = maxv;
    vertex_data[2] = maxu;
    vertex_data[3] = maxv;
    vertex_data[4] = minu;
    vertex_data[5] = minv;
    vertex_data[6] = maxu;
    vertex_data[7] = minv;

    for (float &po : vertex_data) {
        po *= 2.0f;
        po -= 1.0f;
    }

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertex_data, &vertex_data, GL_STATIC_DRAW);
}

GlesRenderer::GlesRenderer(SDL_Window *window, SDL_Texture *texture, const float input_size[2], const float output_size[2], const float sharpness) {
#if defined(IOS) || defined(ANDROID)
//#define SDL_PROC(ret,func,params) func=func;
#else
#define SDL_PROC(ret,func,params) \
    do { \
        func = utils::auto_cast(SDL_GL_GetProcAddress(#func)); \
        if ( ! func ) { \
            SDL_SetError("Couldn't load GLES2 function %s: %s", #func, SDL_GetError()); \
            utils::printError("Couldn't load GLES2 function %s: %s", #func, SDL_GetError()); \
        } \
    } while ( 0 );
#include "gles2funcs.h"
#undef SDL_PROC
#endif
    this->window = window;
    this->texture = texture;
    SDL_GL_BindTexture(texture, nullptr, nullptr);
    context = SDL_GL_GetCurrentContext();
    vert_shader = createShader(GL_VERTEX_SHADER, post_vert_src);
    frag_shader = createShader(GL_FRAGMENT_SHADER, post_cas_glsl);
    const auto id = glCreateProgram();
    glAttachShader(id, vert_shader);
    glAttachShader(id, frag_shader);
    glBindAttribLocation(id, 0, "a_position");
    glBindAttribLocation(id, 1, "a_texCoord");
    glLinkProgram(id);
    GLint link_successful;
    glGetProgramiv(id, GL_LINK_STATUS, &link_successful);
    if (!link_successful) {
        char info_log[256];
        glGetProgramInfoLog(id, sizeof info_log, nullptr, info_log);
        utils::printError("Failed to link shader program:\n%s\n", info_log);
        glDeleteProgram(id);
        exit(-1);
    }
    post_program = id;
    const_buffer_location[0] = glGetUniformLocation(id, "Const0");
    const_buffer_location[1] = glGetUniformLocation(id, "Const1");
    const_buffer_location[2] = glGetUniformLocation(id, "Const2");

    setConstBuffer(input_size, output_size, sharpness);
    initVertexData();
}

GlesRenderer::~GlesRenderer() {
    glDeleteProgram(post_program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

GLuint GlesRenderer::createShader(const GLenum shader_type, const GLchar *shader_src) {
    const GLuint id = glCreateShader(shader_type);
    glShaderSource(id, 1, &shader_src, nullptr);
    glCompileShader(id);
    GLint compiled ;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char info_log[256];
        glGetShaderInfoLog(id, sizeof info_log, nullptr, info_log);
        utils::printError("Failed to load the shader:\n%s\n", info_log);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

void GlesRenderer::copy(const int window_x, const int window_y) {
    if (_pause) return;

    if (SDL_GL_GetCurrentContext() != context && SDL_GL_MakeCurrent(window, context) < 0) return;
    glActiveTexture(GL_TEXTURE0);
    SDL_GL_BindTexture(texture, nullptr, nullptr);
    glViewport(window_x, -window_y, output_size[0], output_size[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glUseProgram(post_program);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glUniform4f(const_buffer_location[0], cas_con[0], cas_con[1], cas_con[2], cas_con[3]);
    glUniform4f(const_buffer_location[1], cas_con[4], cas_con[5], cas_con[6], cas_con[7]);
    glUniform4f(const_buffer_location[2], window_x, window_y, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    GLES_CHECK_ERROR("copy")
}

void GlesRenderer::pause() {
    _pause = true;
}

void GlesRenderer::resume() {
    _pause = false;
}
