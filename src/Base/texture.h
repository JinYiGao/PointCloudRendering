/*
 * @Descripttion: 自己创建的纹理类 用于一些简单的处理
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-09 21:35:40
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-09 21:36:03
 */
#pragma once
#include <vector>
#include <QOpenGLFunctions_4_5_Core>
#include <Base/base.h>

class Texture: protected QOpenGLFunctions_4_5_Core
{
public:
	GLuint ID;//纹理ID引用
	GLenum type; //纹理类型
	GLint width;
	GLint height;

public:
	Texture();
	//创建一个空纹理
	Texture(GLint width, GLint height, GLint internalformat = GL_RGB);
	//从图片创建纹理
	Texture(const char *filename);
	//创建立方体贴图
	GLuint createCubeMap(std::vector<std::string> faces);

	//绑定该纹理
	void bind();
	void bind(uint index);
	//解绑该纹理
	void release();

	//设置纹理样式
	void setTexParameteri(GLenum pname, GLint param);
	
	//释放该纹理句柄
	void destroy();
};