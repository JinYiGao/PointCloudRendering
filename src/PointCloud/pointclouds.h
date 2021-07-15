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

class PointCloud {
public:
	// name
	std::string name;

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

	// 点云编辑平移值
	Eigen::Vector3f translate;

	// 点云编辑旋转值
	Eigen::Matrix4f rotation;

	// 点云编辑尺度缩放
	float zoom;

public:
	PointCloud();

	Eigen::Matrix4f getModelMatrix();
};