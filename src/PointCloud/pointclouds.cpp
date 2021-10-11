/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:34:20
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-05-29 18:34:20
 */
#include <pointclouds.h>
#include <Base/utils.h>

PointCloud::PointCloud() {
	this->translate = Eigen::Vector3f::Identity();
	this->rotation = Eigen::Matrix4f::Identity();
	this->zoom = 1.0;
}

PointCloud::~PointCloud() {
	delete indexTable;
}

// 获取点云转换矩阵
Eigen::Matrix4f PointCloud::getModelMatrixToOrigin() {
	// 自动平移至中心位置
	auto transTocenter = Eigen::Affine3f(Eigen::Translation<float, 3>(-boundingBox.center())).matrix();
	// 用户定义平移量
	auto trans = Eigen::Affine3f(Eigen::Translation<float, 3>(this->translate)).matrix();
	//先旋转 后平移
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

bool PointCloud::isempty() {
	return (points_num == 0);
}

// 在原始点云上添加一块点云
void PointCloud::addPcd(std::shared_ptr<PointCloud> &pcd) {
	this->flagTable.push(this->position.cols()); // 记录添加标记位
	// 合并点云信息
	this->position = mergeMatrixLeftRight(this->position, pcd->position);
	this->colors = mergeMatrixLeftRight(this->colors, pcd->colors);
	this->labels = mergeMatrixLeftRight(this->labels, pcd->labels);
	this->intensity = mergeMatrixLeftRight(this->intensity, pcd->intensity);

	// boundingBox
	Eigen::Vector3f minp = this->position.rowwise().minCoeff(); // 取出每一列中最小值 得到最小点
	Eigen::Vector3f maxp = this->position.rowwise().maxCoeff(); // 取出每一列中最大值 得到最大点
	this->boundingBox.extend(minp);
	this->boundingBox.extend(maxp);

	// pointsNum
	this->points_num = this->points_num + pcd->points_num;

	// Max Min Idensity
	this->setmaxIdensity(this->intensity.rowwise().maxCoeff()(0, 0));
	this->setminIdensity(this->intensity.rowwise().minCoeff()(0, 0));

	// 分配 indexTable
	this->indexTable = new uint32_t[this->points_num];
}

// 删除上一次添加的点云
void PointCloud::removeLastPcd() {
	if (this->flagTable.isEmpty()) {
		return;
	}
	// 出栈
	int flag = this->flagTable.pop();

	// Resize提取
	this->position.conservativeResize(this->position.rows(), flag);
	this->colors.conservativeResize(this->colors.rows(), flag);
	this->labels.conservativeResize(this->labels.rows(), flag);
	this->intensity.conservativeResize(this->intensity.rows(), flag);

	// boundingBox
	Eigen::Vector3f minp = this->position.rowwise().minCoeff(); // 取出每一列中最小值 得到最小点
	Eigen::Vector3f maxp = this->position.rowwise().maxCoeff(); // 取出每一列中最大值 得到最大点
	this->boundingBox.extend(minp);
	this->boundingBox.extend(maxp);

	// pointsNum
	this->points_num = flag;

	// Max Min Idensity
	this->setmaxIdensity(this->intensity.rowwise().maxCoeff()(0, 0));
	this->setminIdensity(this->intensity.rowwise().minCoeff()(0, 0));

	// 分配 indexTable
	this->indexTable = new uint32_t[this->points_num];
}