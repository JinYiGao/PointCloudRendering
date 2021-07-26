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
	GLuint fbo; //framebuffer引用
	vector<GLuint> textures; // 纹理附件 
	int numColorAttachments = 0; // 颜色纹理附件数
	GLuint depth; // 深度缓冲纹理附件, 额外提出来

	float width = 0;
	float height = 0;
	int samples = 1; // 多重采样样本数

public:
	FrameBuffer();

	void bind(); // 绑定到当前framebuffer

	void setNumColorAttachments(int numColorAttachments); // 设置需要的颜色附件数量
	void setSamples(int samples); // 设置样本数
	void setSize(float width, float height); // 设置帧缓冲大小
	void updateBuffers(); // 根据当前帧缓冲 类中设置信息 更新帧缓冲状态
};