/*
 * @Descripttion: 一些简单的OpenGL的结构封装
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-12 22:29:51
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-12 22:30:08
 */
#pragma once

#include <QOpenGlFunctions_4_5_Core>


 // Uniform Block 管理 块布局 默认std140
struct UniformBlock : protected QOpenGLFunctions_4_5_Core {
	std::string name;
	GLuint buffer;

	UniformBlock() {

	}

	UniformBlock(std::string name) {
		initializeOpenGLFunctions();

		this->name = name;
		glCreateBuffers(1, &buffer);// 创建uniform block缓冲区
	}
	~UniformBlock() {
		glDeleteBuffers(1, &buffer);
	}
};


// 根据OpenGL Type编码获取类型占字节
static size_t TypeSize(GLenum type) {
	size_t size;

#define CASE(Enum, Count, Type) \
case Enum: size = Count * sizeof(Type); break

	switch (type) {
		CASE(GL_FLOAT, 1, GLfloat);
		CASE(GL_FLOAT_VEC2, 2, GLfloat);
		CASE(GL_FLOAT_VEC3, 3, GLfloat);
		CASE(GL_FLOAT_VEC4, 4, GLfloat);
		CASE(GL_INT, 1, GLint);
		CASE(GL_INT_VEC2, 2, GLint);
		CASE(GL_INT_VEC3, 3, GLint);
		CASE(GL_INT_VEC4, 4, GLint);
		CASE(GL_UNSIGNED_INT, 1, GLuint);
		CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
		CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
		CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
		CASE(GL_BOOL, 1, GLboolean);
		CASE(GL_BOOL_VEC2, 2, GLboolean);
		CASE(GL_BOOL_VEC3, 3, GLboolean);
		CASE(GL_BOOL_VEC4, 4, GLboolean);
		CASE(GL_FLOAT_MAT2, 4, GLfloat);
		CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
		CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
		CASE(GL_FLOAT_MAT3, 9, GLfloat);
		CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
		CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
		CASE(GL_FLOAT_MAT4, 16, GLfloat);
		CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
		CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
#undef CASE
	default:
		fprintf(stderr, "Unknown type: 0x%x\n", type);
		exit(EXIT_FAILURE);
		break;
	}

	return size;
}