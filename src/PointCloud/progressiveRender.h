/*
 * @Descripttion: 渐进式点云渲染算法
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-19 15:44:23
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-19 15:44:45
 */

#pragma once
#include <iostream>
#include <QOpenGlFunctions_4_5_Core>

#include <Base/shader.h>
#include <Base/texture.h>
#include <Base/framebuffer.h>
#include <Base/glbuffer.h>
#include <Base/camera.h>
#include <Base/glutils.h>

#include <PointCloud/pointclouds.h>
#include <PointCloud/uploader.h>

class RenderWidget;
struct Point;
struct Window;
// 渐进渲染 渲染状态管理(一些可能经常变的量)
struct ProgressiveRenderState : protected QOpenGLFunctions_4_5_Core {
	GLBuffer *reprojectBuffer; // 重投影数据缓冲管理
	GLuint IndirectCommand; // 间接绘制命令参数缓存
	int fillOffset; // 空洞填充偏移索引

	ProgressiveRenderState() {
		initializeOpenGLFunctions();
		// 重投影数据缓冲管理
		reprojectBuffer = new GLBuffer();
		int vboCapacity = 3000000;
		int vboBytes = vboCapacity * 20;
		vector<GLBufferAttribute> repAttributes;
		GLBufferAttribute repAttribute1("position", 0, 3, GL_FLOAT, GL_FALSE, 24, 0);
		repAttributes.emplace_back(repAttribute1);
		GLBufferAttribute repAttribute2("color", 1, 1, GL_INT, GL_FALSE, 24, 12);
		repAttributes.emplace_back(repAttribute2);
		GLBufferAttribute repAttribute3("index", 2, 1, GL_INT, GL_FALSE, 24, 16);
		repAttributes.emplace_back(repAttribute3);
		GLBufferAttribute repAttribute4("show", 3, 1, GL_INT, GL_FALSE, 24, 20);
		repAttributes.emplace_back(repAttribute4);

		reprojectBuffer->setEmptyInterleaved(repAttributes, GLsizei(vboBytes));

		// 间接绘制命令缓冲
		glCreateBuffers(1, &IndirectCommand);
		int commandBytes = 5 * 4;
		glNamedBufferData(IndirectCommand, GLsizei(commandBytes), NULL, GL_DYNAMIC_DRAW);

		fillOffset = 0;
	}
};

class ProgressiveRender : protected QOpenGLFunctions_4_5_Core {
public:
	ProgressiveRender(RenderWidget *glWidget, PointCloud *pcd);
	~ProgressiveRender();

public:
	std::string name;

public:
	// 1 渐进方式渲染
	void renderPointCloudProgressive();

	void Segment(vector<Point> polygon); // 控制裁剪

protected:
	// 初始化上载点云
	void init();
	// --- 三大核心步骤
	void reproject();
	void fillFixed();
	void createVBO();

	ProgressiveRenderState* getRenderState(); // 获取渐进方式渲染状态信息

	void uploadAttribute(); // 上传更新属性信息

protected:
	RenderWidget *glWidget = nullptr;
	FrameBuffer *fbo = nullptr; // 离屏绘制到的帧缓冲
	Camera *camera = nullptr;
	Window *window = nullptr;

	PointCloud *pcd = nullptr; // 点云信息
	UpLoader *upload = nullptr; // 点云数据上载GPU模块

	vector<GLBuffer*> pointcloudBuffers; // 大型点云分块缓冲管理

	ProgressiveRenderState *renderState = nullptr; // 渐进渲染的状态管理

	Texture *gradientImage = nullptr; // 颜色梯度纹理, 用于着色器内获取颜色

	Shader *reprojectShader = nullptr; // 重投影着色器
	Shader *fillShader = nullptr; // 空洞填充着色器
	Shader *createVBOShader = nullptr; // 创建重投影VBO着色器

	Shader *SegmentShader = nullptr; // 裁剪着色器

	vector<UniformBlock*> uniformBlocks; // uniform块管理

	int currentAttributeMode = -1;
	// ------------------------------------------------------------------------------------------------------------------------------- 
};