/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:34:20
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-05-29 18:34:20
 */
#include <pointclouds.h>

PointCloud::PointCloud() {
	this->translate = Eigen::Vector3f::Identity();
	this->rotation = Eigen::Matrix4f::Identity();
	this->zoom = 1.0;
}

// 获取点云转换矩阵
Eigen::Matrix4f PointCloud::getModelMatrix() {
	// 自动平移至中心位置
	auto transTocenter = Eigen::Affine3f(Eigen::Translation<float, 3>(-boundingBox.center())).matrix();
	// 用户定义平移量
	auto trans = Eigen::Affine3f(Eigen::Translation<float, 3>(this->translate)).matrix();
	//先旋转 后平移
	Eigen::Matrix4f model = trans * rotation * Eigen::Affine3f(Eigen::Scaling(Eigen::Vector3f::Constant(zoom))).matrix() * transTocenter;
	return model;
}