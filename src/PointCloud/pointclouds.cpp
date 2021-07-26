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

PointCloud::~PointCloud() {

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

void PointCloud::setVisible(bool show) {
	this->show = show;
}

bool PointCloud::getVisible() const {
	return show;
}

void PointCloud::setAttributeMode(int mode) {
	this->ATTRIBUTE_MODE = mode;
}

int PointCloud::getAttributeMode() const {
	return ATTRIBUTE_MODE;
}
void PointCloud::setPointSize(float pointSize) {
	this->pointSize = pointSize;
}

float PointCloud::getPointSize() const {
	return pointSize;
}

void PointCloud::setColorStrip(int steps, Eigen::MatrixXf colorStrip) {
	this->colorStrip.resize(4, steps);
	this->colorStrip = colorStrip;
}

void PointCloud::setmaxIdensity(int maxIdensity) {
	this->maxIdensity = maxIdensity;
}

void PointCloud::setminIdensity(int minIdensity) {
	this->minIdensity = minIdensity;
}

void* PointCloud::getColorStrip() const {
	return (void*)this->colorStrip.data();
}

int PointCloud::getmaxIdensity() const {
	return maxIdensity;
}

int PointCloud::getminIdensity() const {
	return minIdensity;
}