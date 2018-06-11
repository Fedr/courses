#pragma once

#include <epoxy/gl.h>

void check_shader(GLuint shader);
GLuint shader_compile(GLenum type, const GLchar* src);
void program_check(GLuint program);
GLuint program_link(GLuint vert, GLuint frag);
void teardown(GLint* viewport);
GLuint texture_new();
void fbo_check(const char* description);

/*
*  Builds a quad covering the viewport, returning the relevant VAO
*/
GLuint quad_new();
