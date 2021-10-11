/*
 * @Descripttion: 点云选择工具 —— 点选 加标记
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-09-10 16:44:24
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-09-10 16:44:25
 */
#pragma once 
#include <Base/tool.h>
#include <vector>
#include <Eigen/Eigen>

using std::vector;

class Shader;
class GLBuffer;
class FrameBuffer;
class PointCloud;
class RenderWidget;
class ProgressiveRender;
struct GLBufferAttribute;

class ToolPick :public Tool {
	Q_OBJECT
public:
	ToolPick(RenderWidget *glWidget);
	~ToolPick();

	// 重写父类虚函数
	void activate(); // 启用工具
	void deactivate(); // 退出工具
	void suspend(); // 暂停
	void resume(); // 恢复
	void reset(); // 重置

	int getToolType();

	void draw(QPainter *painter); // QPainter Draw
	void gl_draw(); // OpenGL方式绘制

	void mousePress(QMouseEvent *e);
	void mouseRelease(QMouseEvent *e);
	void mouseMove(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);
	void keyPress(QKeyEvent *e);

protected:
	int toolType = PickPointTool;
	bool activated;
	RenderWidget *glWidget = nullptr;
	
	Shader *drawPointShader = nullptr;
	Eigen::Vector3f point = Eigen::Vector3f::Zero();
	GLBuffer *glbuffer = nullptr;
	vector<GLBufferAttribute> glBufferAttributes;
};