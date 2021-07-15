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
#include <PointCloud/pointclouds.h>
#include <PointCloud/uploader.h>

// 视口大小
struct Window {
	int width;
	int height;
};

struct Point {
	float x;
	float y;
	Point(float x, float y) {
		this->x = x;
		this->y = y;
	}
};

// 渐进渲染 渲染状态管理(一些可能经常变的量)
struct ProgressiveRenderState: protected QOpenGLFunctions_4_5_Core {
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

//继承自QOpenGLFunctions可以避免每次调用opengl函数时使用前缀
class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core{
public:
	RenderWidget(QWidget *parent);
	RenderWidget(QWidget *parent, PointCloud *pcd);

	Window window;
	int fps = 0;
	
    Camera *camera = nullptr; // 相机
	PointCloud *pcd = nullptr; // 点云信息
	UpLoader *upload = nullptr; // 点云数据上载GPU模块

	int MSAA_SAMPLES = 1; // 多重采样抗锯齿
	bool EDL_ENABLE = true; // 是否开启EDL
	int ATTRIBUTE_MODE = 1; // 着色器内颜色读取模式 0: 从纹理采样 , 1: 由数据本身获取 , 2: 按分类着色 , 3: 按强度着色 , 4: 按RGB着色
	float pointSize = 1.5; // 点大小
	// 色带相关设置
	int maxValue;
	int minValue;
	Eigen::MatrixXf colorStrip;

// OpenGL绘制相关资源 
public:
	FrameBuffer *fbo = nullptr; // 离屏绘制到的帧缓冲
	FrameBuffer *fboEDL = nullptr; // EDL处理后帧缓冲
	GLBuffer *quadBuffer = nullptr; // 离屏渲染后纹理贴图quad缓冲管理

	vector<GLBuffer*> pointcloudBuffers; // 大型点云分块缓冲管理

	ProgressiveRenderState *renderState = nullptr; // 渐进渲染的状态管理

	Texture *gradientImage = nullptr; // 颜色梯度纹理, 用于着色器内获取颜色

	Shader *reprojectShader = nullptr; // 重投影着色器
	Shader *fillShader = nullptr; // 空洞填充着色器
	Shader *createVBOShader = nullptr; // 创建重投影VBO着色器

	Shader *edlShader = nullptr; // EDL处理着色器
	Shader *edlShaderMSAA = nullptr; //EDL处理多重采样抗锯齿着色器

	Shader *SegmentShader = nullptr; // 裁剪着色器

	vector<UniformBlock*> uniformBlocks; // uniform块管理

public:
	void init(PointCloud *pcd); // 初始化点云数据

	void startDrawPolygon();
	void endDrawPolygon();
	void polygonSegement(int mode); // 多边形裁剪
	void checkSegement(); // 确定裁剪 提取裁出来的点
	void cancelSegement(); // 取消裁剪

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

	// 鼠标操作
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

private:
	bool segement = false; // 是否确定裁剪
	bool segementMode = -1; // 裁剪模式 0: inpolygon 1: outpolygon
	bool enableDraw = false; // 绘制工具是否启用
	bool drawingPolygon = false; // 是否正在绘制
	vector<Point> polygon;
	QPainter *painter = nullptr;

	void Segement();

private:
	// 1 渐进方式渲染
	void renderPointCloudProgressive();
	// --- 三大核心步骤
	void reproject();
	void fillFixed();
	void createVBO();

	ProgressiveRenderState* getRenderState(); // 获取渐进方式渲染状态信息
};