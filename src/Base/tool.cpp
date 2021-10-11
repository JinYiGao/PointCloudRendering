/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-17 15:24:48
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-17 15:24:48
 */
#include <tool.h>

ToolManager::ToolManager(QObject *parent) : QObject(parent) {
	this->currentTool = nullptr;
	this->Suspended = false;
}

ToolManager::~ToolManager() {

}

Tool* ToolManager::get_tool(int id) const {
	return tools.at(id);
}

Tool* ToolManager::get_current() const {
	return currentTool;
}

int ToolManager::getToolType()const {
	return currentTool->getToolType();
}

// 暂停当前工具使用
void ToolManager::suspend_current() {
	if (currentTool) {
		currentTool->suspend();
	}
	Suspended = true;
}

// 恢复当前工具使用
void ToolManager::resume_current() {
	if (currentTool) {
		currentTool->resume();
	}
	Suspended = false;
}

// 重置当前工具
void ToolManager::reset_current() {
	if (currentTool) {
		currentTool->reset();
	}
}

bool ToolManager::isSuspended() const {
	return this->Suspended;
}

void ToolManager::register_tool(int id, Tool *tool) {
	this->tools.emplace(id, tool);
}

// 移除当前管理的所有工具
void ToolManager::removeAll() {
	for (auto &pair : tools) {
		delete pair.second;
		pair.second = nullptr;
	}
	tools.clear();
}

// 更改当前工具
void ToolManager::changeTool(int id) {
	// 先退出当前工具
	if (currentTool) {
		currentTool->deactivate();
	}
	// 判断待使用的工具是否存在
	if (tools.count(id)) {
		currentTool = tools.at(id);
		currentTool->activate();
	}
	else {
		currentTool = nullptr;
	}
}

// 绘制
void ToolManager::draw(QPainter *painter) {
	if (currentTool) {
		currentTool->draw(painter);
	}
}

void ToolManager::gl_draw() {
	if (currentTool) {
		currentTool->gl_draw();
	}
}

// 键鼠操作
void ToolManager::mousePress(QMouseEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->mousePress(e);
	}
}

void ToolManager::mouseRelease(QMouseEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->mouseRelease(e);
	}
}

void ToolManager::mouseMove(QMouseEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->mouseMove(e);
	}
}

void ToolManager::mouseDoubleClick(QMouseEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->mouseDoubleClick(e);
	}
}

void ToolManager::wheelEvent(QWheelEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->wheelEvent(e);
	}
}

void ToolManager::keyDown(QKeyEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->keyDown(e);
	}
}

void ToolManager::keyUp(QKeyEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->keyUp(e);
	}
}

void ToolManager::keyPress(QKeyEvent *e) {
	if (currentTool && !Suspended) {
		currentTool->keyPress(e);
	}
}

Tool::Tool() { 
	setParent(nullptr); 
}

Tool::~Tool() {};
