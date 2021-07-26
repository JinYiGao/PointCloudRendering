/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:36:32
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-01 14:47:02
 */
#pragma once

#include <iostream>

#include <qevent.h>
#include <QOpenGLWidget>
#include <QOpenGlFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QPoint>

#include <Base/utils.h>
#include <Base/shader.h>
#include <Base/texture.h>
#include <Base/framebuffer.h>
#include <Base/glbuffer.h>
#include <Base/camera.h>
#include <Base/glutils.h>
#include <Base/tool.h>
#include <Base/tool_camera.h>
#include <Base/tool_drawPolygon.h>
#include <PointCloud/pointclouds.h>
#include <PointCloud/uploader.h>
#include <PointCloud/progressiveRender.h>

// 视口大小
struct Window {
	int width;
	int height;
};

//继承自QOpenGLFunctions可以避免每次调用opengl函数时使用前缀
class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core{
public:
	RenderWidget(QWidget *parent);
	RenderWidget(QWidget *parent, PointCloud *pcd);
	~RenderWidget();

	Window *window = new Window();
	int fps = 0;

	Camera *camera = nullptr; // 相机

	// ------------------------------------- Tool ----------------------------------------
	ToolManager *toolManager = nullptr; // 工具管理
	ToolCamera *toolCamera = nullptr; // 三维相机工具
	ToolDrawPolygon *toolDrawPolygon = nullptr; // 多边形绘制工具

// OpenGL绘制相关资源 
public:
	FrameBuffer *fbo = nullptr; // 离屏绘制到的帧缓冲
	FrameBuffer *fboEDL = nullptr; // EDL处理后帧缓冲
	GLBuffer *quadBuffer = nullptr; // 离屏渲染后纹理贴图quad缓冲管理

	Texture *gradientImage = nullptr; // 颜色梯度纹理, 用于着色器内获取颜色

	Shader *reprojectShader = nullptr; // 重投影着色器
	Shader *fillShader = nullptr; // 空洞填充着色器
	Shader *createVBOShader = nullptr; // 创建重投影VBO着色器

	Shader *edlShader = nullptr; // EDL处理着色器
	Shader *edlShaderMSAA = nullptr; //EDL处理多重采样抗锯齿着色器

	Shader *SegmentShader = nullptr; // 裁剪着色器

	vector<UniformBlock*> uniformBlocks; // uniform块管理

public:
	void addPointCloud(PointCloud *pcd); // 添加显示点云

	void setMSAAsamples(int samples);
	void setEDL(bool enable);

	void startSegment(vector<std::string> name); // 对指定name点云进行裁剪 

protected:
	vector<ProgressiveRender*> renderList; // 渲染集合

	int MSAA_SAMPLES = 1; // 多重采样抗锯齿
	bool EDL_ENABLE = true; // 是否开启EDL

protected:
	void initTools();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

	// 鼠标操作
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

private:
	QPainter *painter = nullptr; // 二维绘制
};