/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-06 16:06:20
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-06 16:06:20
 */
#include <Base/shader.h>

Shader::Shader() {
	initializeOpenGLFunctions();
} 

Shader::~Shader() {
	glDeleteBuffers(1, &programId);
	delete shader;
}

// 创建计算着色器程序 
Shader::Shader(const QString &computerShaderFileName) {
	initializeOpenGLFunctions();
	shader = new QOpenGLShaderProgram();
	shader->addShaderFromSourceFile(QOpenGLShader::Compute, computerShaderFileName);
	bool success = shader->link();
	if (!success)
	{
		qDebug() << shader->log();
	}
	else
	{
		programId = shader->programId();
		qDebug() << "Success Link computerShader";
	}
}

// 根据顶点着色器与片段着色器 创建着色器程序
Shader::Shader(const QString &vertShaderFileName, const QString &fragShaderFilename)
{
	initializeOpenGLFunctions();
	shader = new QOpenGLShaderProgram();
	shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertShaderFileName);
	shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragShaderFilename);
	bool success = shader->link();
	if (!success)
	{
		qDebug() << shader->log();
	}
	else
	{
		programId = shader->programId();
		qDebug() << "Success Link Shader";
	}
}

bool Shader::compileShaderFromSourceCode(const char *vertShaderSource, const char *fragShaderSource) {
	shader = new QOpenGLShaderProgram();
	shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertShaderSource);
	shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragShaderSource);
	bool success = shader->link();
	if (!success)
	{
		qDebug() << shader->log();
	}
	else
	{
		programId = shader->programId();
		qDebug() << "Success Link Shader";
	}
	return success;
}

bool Shader::bind()
{
	return shader->bind();
}

void Shader::unbind()
{
	shader->release();
}

//void Shader::setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z)
//{
//	shader->setUniformValue(name, x, y, z);
//}
