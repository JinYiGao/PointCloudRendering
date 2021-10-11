/*
 * @Descripttion: OpenGL VAO VBO 缓冲管理类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-05 16:31:36
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-05 16:31:37
 */
#pragma once

#include <iostream>
#include <vector>
#include <QOpenGLFunctions_4_5_Core>

#include <Base/common.h>

using std::vector;

// 顶点属性解析结构体
struct GLBufferAttribute {
	std::string name;
	int location; // 属性绑定位 对应着色器location
	int count; // 读取指定数量的数据
	GLenum type; // 读取的数据类型
	GLboolean normalize;
	GLsizei stride; // 每隔多少字节读取
	GLsizei offset; // 首指针位置 偏移offset个字节

	GLBufferAttribute(std::string name, int location, int count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset) {
		this->name = name;
		this->location = location;
		this->count = count;
		this->type = type;
		this->normalize = normalize;
		this->stride = stride;
		this->offset = offset;
	}
};

class GLBuffer : protected QOpenGLFunctions_4_5_Core {
public:
	GLuint VAO;
	GLuint VBO;
	int vertexCount = 0; // 顶点数量 或 需绘制的顶点数

	vector<GLBufferAttribute> attributes; //顶点属性解析

public:
	GLBuffer();
	GLBuffer(GLuint VAO, GLuint VBO);
	GLBuffer(GLuint VBO);
	~GLBuffer();

	// 绑定该 Buffer 进行绘制
	void bind();

	// 传入顶点数据并设置交错顶点属性解析
	// 传入数组
	void set(const void *vertices, GLsizei size, vector<GLBufferAttribute> attributes, int count);
	// 以Eigen矩阵方式传入
	template<typename T>
	void set(Eigen::Matrix<T, -1, -1> &vertices, GLsizei size, vector<GLBufferAttribute> attributes, int count) {
		this->attributes = attributes;
		this->vertexCount = count;

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//std::cout << vertices.size() * sizeof(*vertices.data()) << std::endl;
		glBufferData(GL_ARRAY_BUFFER, size, vertices.data(), GL_DYNAMIC_DRAW);

		setInterleaved(attributes);
	}

	// 开辟空的顶点缓冲区并设置交错顶点属性解析
	void setEmptyInterleaved(vector<GLBufferAttribute> attributes, GLsizei size);

	// --- 还有一种分块顶点数据解析 暂未集成---

	void setInterleaved(vector<GLBufferAttribute> attributes); // 设置交错顶点属性
};