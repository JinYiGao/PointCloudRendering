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
};