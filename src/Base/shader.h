/*
 * @Descripttion: 自己的shader类 封装了 QOpenGLShaderProgram 的一些方法
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-06 16:06:15
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-06 16:06:49
 */
#pragma once
#include <iostream>
#include <vector>

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_5_Core>
#include <Eigen/Eigen>
#include <Base/utils.h>
#include <Base/glutils.h>

using std::vector;

class Shader : protected QOpenGLFunctions_4_5_Core
{
public: 
	QOpenGLShaderProgram *shader;//着色器程序
	GLuint programId; //着色器程序ID
	
public:
	Shader();
	~Shader();

	Shader(const QString &computerShaderFileName);

	Shader(const QString &vertShaderFileName,const QString &fragShaderFilename);

	bool compileShaderFromSourceCode(const char *vertShaderSource, const char *fragShaderSource);

	///绑定着色器
	bool bind();
	///解绑着色器
	void unbind();

	///setUniform Matrix4x4
	template <typename T>
	void setUniformValue(const char *name, const Eigen::Matrix<T, 4, 4> &mat)
	{
		shader->bind();
		shader->setUniformValue(name, QMatrix4x4(mat.template cast<float>().data(), 4, 4));
	}

	template <typename T>
	void setUniformValue(const char *name, const Eigen::Matrix<T, 3, 1> &vector)
	{
		shader->bind();
		shader->setUniformValue(name, vector.x(), vector.y(), vector.z());
	}

	///setUniform x,y,z
	void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z)
	{
		shader->bind();
		shader->setUniformValue(name, x, y, z);
	}

	///setUniform int float...
	template <typename T>
	void setUniformValue(const char *name, T data)
	{
		shader->bind();
		shader->setUniformValue(name, data);
	}

	// 设置uniform块变量
	//template <typename T>
	/*typename std::enable_if <std::is_same<int, T>::value || std::is_same<float, T>::value || std::is_same<uint, T>::value, void >::type*/
	void setUniformBlockValue(vector<UniformBlock*> &uniformBlocks, const char *BlockName, const char *valueName, void* value) {
		UniformBlock *ubo = nullptr;
		GLint uboSize;
		GLuint uboIndex;
		GLint ubobinding;
		bool find = false;
		for (int i = 0; i < uniformBlocks.size(); i++) {
			if (uniformBlocks[i]->name == BlockName) {
				ubo = uniformBlocks[i];
				find = true;
				break;
			}
		}
		if (!find) {
			ubo = new UniformBlock(BlockName);
			uboIndex = glGetUniformBlockIndex(this->programId, BlockName);
			// 查询ubiform块大小
			glGetActiveUniformBlockiv(this->programId, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
			// 分配空间
			glNamedBufferData(ubo->buffer, uboSize, nullptr, GL_DYNAMIC_DRAW);

			uniformBlocks.emplace_back(ubo);
		}
	
		uboIndex = glGetUniformBlockIndex(this->programId, BlockName);
		// 查询uniform block绑定位
		glGetActiveUniformBlockiv(this->programId, uboIndex, GL_UNIFORM_BLOCK_BINDING, &ubobinding);
		// 绑定该缓冲到着色器目标点
		glBindBufferBase(GL_UNIFORM_BUFFER, ubobinding, ubo->buffer);

		// 查找变量索引
		GLuint valueIndex;
		glGetUniformIndices(this->programId, 1, &valueName, &valueIndex);
		// 获取变量偏移位置 大小 类型等
		GLint offset, size, type;
		glGetActiveUniformsiv(this->programId, 1, &valueIndex, GL_UNIFORM_OFFSET, &offset);
		glGetActiveUniformsiv(this->programId, 1, &valueIndex, GL_UNIFORM_SIZE, &size);
		glGetActiveUniformsiv(this->programId, 1, &valueIndex, GL_UNIFORM_TYPE, &type);
		// 写入数据 根据 偏移+大小
		GLsizei valueSize = size * TypeSize(type);
		glNamedBufferSubData(ubo->buffer, offset, valueSize, value);
	}
};