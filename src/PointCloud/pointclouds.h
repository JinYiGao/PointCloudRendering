/*
 * @Descripttion: 点云类 --用于组织管理点云数据
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:34:05
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-06-30 16:07:04
 */
#pragma once
#include <Base/common.h>
#include <memory>
#include <QString>
#include <QStack>
#include <vector>

using std::vector;

enum ATTRIBUTEMODE {
	FROM_GRADIENT = 0, // 从颜色梯度图采样
	FROM_RBG, // 从RGB采样
	FROM_Label, //从分类采样
	FROM_INTENSITY, // 从强度着色
	NO_COLOR // 不展示颜色
};

// 分类标签
enum CLASSIFICATION {
	NeverClassified = 0,
	Unassigned,
	Ground,
	LowVegetation,
	MediumVegetation,
	HighVegetation,
	Building,
	Noise,
	Reserved,
	Water,
	Rail,
	Road,
	Overlap,
	WireGuard,
	WireConductor,
	TransmissionTower,
	WireConnector,
	BridgeDeck,
	HighNoise
};

class PointCloud {
public:
	// name
	QString name;
	
	// ---------------------------------------------------------------------------------------
	// ----------------------------------- Attributes ----------------------------------------
	// ---------------------------------------------------------------------------------------

	int points_num = 0;

	// 位置矩阵
	// 3 * n 因为eigen为列主序 存储缓冲区时 指针移动按列存储
	//----------------------
	// x x x ...
	// y y y ...
	// z z z ...
	//----------------------
	Eigen::MatrixXf position;

	// 颜色矩阵
	// 4 * n
	// r r r ...
	// g g g ...
	// b b b ...
	// a a a ...
	Eigen::MatrixXi_8 colors;

	// 分类信息
	// 1 * n
	Eigen::MatrixXi labels;

	// 强度信息
	// 1 * n
	Eigen::MatrixXi intensity;

	//包围盒
	BoundingBox3f boundingBox;

	// 点云初始偏移值 ---由las文件读取 offset + position才是世界坐标
	Eigen::Vector3f offset;

	// ---------------------------------------------------------------------------------------

	// 点云编辑平移值
	Eigen::Vector3f translate;

	// 点云编辑旋转值
	Eigen::Matrix4f rotation;

	// 点云编辑尺度缩放
	float zoom;

	uint32_t *indexTable; // 打乱后索引映射 targetIndex —— originIndex

protected:
	//--------------------------------------- 点云样式相关设置 ----------------------------------
	bool show = true;
	int ATTRIBUTE_MODE = 1; // 着色器内颜色读取模式 0: 从纹理采样 , 1: 由数据本身获取 , 2: 按分类着色 , 3: 按强度着色, 4: 不着色
	float pointSize = 2; // 点大小
	// 色带相关设置
	int maxIdensity;
	int minIdensity;
	Eigen::MatrixXf colorStrip;

	QStack<int> flagTable; // 点云添加标记位, 新增点云的起始位置索引

public:
	PointCloud();
	~PointCloud();

	void setVisible(bool show);
	bool getVisible() const;
	void setAttributeMode(int mode);
	int getAttributeMode() const;
	void setPointSize(float pointSize);
	float getPointSize() const;
	void setColorStrip(int steps, Eigen::MatrixXf colorStrip);
	void setmaxIdensity(int maxIdensity);
	void setminIdensity(int minIdensity);
	void* getColorStrip() const;
	int getmaxIdensity() const;
	int getminIdensity() const;

	void removeByIndex(int index); // 按照点索引删除点
	void addPcd(std::shared_ptr<PointCloud> &pcd); // 添加一块点云
	void removeLastPcd(); // 删除上次添加的点云

	Eigen::Matrix4f getModelMatrixToOrigin();

	bool isempty();
};