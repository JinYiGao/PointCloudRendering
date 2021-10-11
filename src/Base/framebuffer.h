/*
 * @Descripttion: FrameBuffer(帧缓冲)管理类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-05 12:02:16
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-05 12:02:17
 */
#pragma once

#include <iostream>
#include <vector>
#include <QOpenGLFunctions_4_5_Core>

using std::vector; 

// -------------------------------------------------------
// 暂未采用渲染缓冲对象附件 ， 所有缓冲均采用纹理附件进行
// -------------------------------------------------------
class FrameBuffer : protected QOpenGLFunctions_4_5_Core {
public:
	GLuint fbo = 0; //framebuffer引用

	float width = 0;
	float height = 0;
	int samples = 1; // 多重采样样本数

	vector<GLuint> textures; // 纹理附件
	GLuint depth; // 深度缓冲纹理附件, 额外提出来

private:
	struct ColorAttachment {
		GLuint buffer; // 作为渲染缓冲对象时
		GLuint texture; // 作为纹理附件时
		GLenum internal_format;
		GLenum format;
		GLenum type;
		GLenum texture_filter;

		ColorAttachment() : buffer(0), texture(0), internal_format(GL_RGBA8), format(GL_RGBA), type(GL_UNSIGNED_BYTE), texture_filter(GL_NEAREST) {}
	};
	vector<ColorAttachment> colorAttachments; // 颜色附件
	int numColorAttachments = 0; // 颜色附件数

	GLuint depth_buffer;
	GLuint depth_texture;
	GLenum depth_internal_format;
	GLenum depth_texture_filter;
	GLenum depth_texture_compare_mode;
	GLenum depth_texture_compare_func;

public:
	FrameBuffer();
	~FrameBuffer();

	void bind(); // 绑定到当前framebuffer

	void setNumColorAttachments(int numColorAttachments); // 设置需要的颜色附件数量
	void setSamples(int samples); // 设置样本数
	void setSize(float width, float height); // 设置帧缓冲大小
	void updateBuffers(); // 根据当前帧缓冲 类中设置信息 更新帧缓冲状态

	// 添加颜色纹理附件
	bool add_color_texture(
		GLenum internal_format = GL_RGBA8, 
		GLenum format = GL_RGBA, 
		GLenum type = GL_UNSIGNED_BYTE, 
		GLenum filter = GL_NEAREST);
};