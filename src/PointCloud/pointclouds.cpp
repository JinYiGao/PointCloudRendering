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

// ��ȡ����ת������
Eigen::Matrix4f PointCloud::getModelMatrix() {
	// �Զ�ƽ��������λ��
	auto transTocenter = Eigen::Affine3f(Eigen::Translation<float, 3>(-boundingBox.center())).matrix();
	// �û�����ƽ����
	auto trans = Eigen::Affine3f(Eigen::Translation<float, 3>(this->translate)).matrix();
	//����ת ��ƽ��
	Eigen::Matrix4f model = trans * rotation * Eigen::Affine3f(Eigen::Scaling(Eigen::Vector3f::Constant(zoom))).matrix() * transTocenter;
	return model;
}