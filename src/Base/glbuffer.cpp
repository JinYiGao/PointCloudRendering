/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-05 16:31:42
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-05 16:31:42
 */

#include <glbuffer.h>

GLBuffer::GLBuffer() {
	initializeOpenGLFunctions();

	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
}

GLBuffer::GLBuffer(GLuint VAO, GLuint VBO) {
	initializeOpenGLFunctions();

	this->VAO = VAO;
	this->VBO = VBO;
}

GLBuffer::GLBuffer(GLuint VBO) {
	initializeOpenGLFunctions();

	glCreateVertexArrays(1, &VAO);
	this->VBO = VBO;
}

void GLBuffer::set(const void *vertices, GLsizei size, vector<GLBufferAttribute> attributes, int count) {
	this->attributes = attributes;
	this->vertexCount = count;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);

	setInterleaved(attributes);
}

void GLBuffer::setEmptyInterleaved(vector<GLBufferAttribute> attributes, GLsizei size) {
	this->attributes = attributes;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);

	setInterleaved(attributes);
}

// 设置顶点交错属性
void GLBuffer::setInterleaved(vector<GLBufferAttribute> attributes) {
	this->attributes = attributes;
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	for (int i = 0; i < attributes.size(); i++) {
		if (attributes[i].type == GL_INT ||
			attributes[i].type == GL_UNSIGNED_INT ||
			attributes[i].type == GL_BYTE ||
			attributes[i].type == GL_UNSIGNED_BYTE) {
			glVertexAttribIPointer(attributes[i].location,
				attributes[i].count,
				attributes[i].type,
				attributes[i].stride,
				(void *)attributes[i].offset
			);
		}
		else {
			glVertexAttribPointer(attributes[i].location,
				attributes[i].count,
				attributes[i].type,
				attributes[i].normalize,
				attributes[i].stride,
				(void *)attributes[i].offset
			);
		}

		glEnableVertexAttribArray(attributes[i].location);
	}

	//解绑VAO
	glBindVertexArray(0);
}

GLBuffer::~GLBuffer() {
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}