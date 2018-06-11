#include "mygl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void check_shader(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint log_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

		GLchar* info_log = (GLchar*)malloc((log_length + 1) * sizeof(GLchar));
		glGetShaderInfoLog(shader, log_length, NULL, info_log);
		fprintf(stderr, "Error: shader failed with error '%s'\n", info_log);
		exit(-1);
	}
}

GLuint shader_compile(GLenum type, const GLchar* src)
{
	assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	check_shader(shader);
	return shader;
}

void program_check(GLuint program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		GLchar* info_log = (GLchar*)malloc((log_length + 1) * sizeof(GLchar));
		glGetProgramInfoLog(program, log_length, NULL, info_log);
		fprintf(stderr, "Error: linking failed with error '%s'\n", info_log);
		exit(-1);
	}
}

GLuint program_link(GLuint vert, GLuint frag)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	program_check(program);
	return program;
}

void teardown(GLint* viewport)
{
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (viewport)
	{
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	}
}

GLuint quad_new()
{
	GLfloat verts[] = { -1.0f, -1.0f,     1.0f, -1.0f,
		1.0f,  1.0f,    -1.0f,  1.0f };
	GLuint vbo;
	GLuint vao;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	return vao;
}

GLuint texture_new()
{
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return tex;
}

void fbo_check(const char* description)
{   /*  Check to see if the framebuffer is complete  */
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Error: %s framebuffer is incomplete (%i)\n",
			description, status);
		exit(-1);
	}
}
