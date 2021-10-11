/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:27:54
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:35:17
 */
#include <pcdManager.h>
#include <Base/utils.h>

PcdManager::PcdManager() {
	std::cout << "Single Manager" << std::endl;
}
PcdManager* PcdManager::m_pInstance = nullptr;

PcdManager::~PcdManager() {
}

bool PcdManager::addPcdToManager(std::shared_ptr<PointCloud> &pcd) {
	pcdMap.emplace(pcd->name, pcd);
	return true;
}

bool PcdManager::remove_pointcloud(std::shared_ptr<PointCloud> &pcd) {
	for (auto it = pcdMap.begin(); it != pcdMap.end();) {
		if (it->first == pcd->name) {
			//delete it->second;
			it->second = nullptr;
			pcdMap.erase(it++); //here is the key
			return true;
		}
		else {
			it++;
		}
	}
	return false;
}

bool PcdManager::remove_pointcloud(QString name) {
	for (auto it = pcdMap.begin(); it != pcdMap.end();) {
		if (it->first == name) {
			//delete it->second;
			it->second = nullptr;
			pcdMap.erase(it++); //here is the key
			return true;
		}
		else {
			it++;
		}
	}
	return false;
}

bool PcdManager::existPcd(QString name) {
	std::map<QString, std::shared_ptr<PointCloud>>::iterator iter = pcdMap.find(name);
	if (iter != pcdMap.end()) {
		return true;
	}
	else {
		return false;
	}
}

std::shared_ptr<PointCloud> PcdManager::getPointCloud(QString name) {
	std::map<QString, std::shared_ptr<PointCloud>>::iterator iter = pcdMap.find(name);
	if (iter != pcdMap.end()) {
		auto pcd = pcdMap.at(name);
		return pcd;
	}
	else{
		return nullptr;
	}
}

std::shared_ptr<PointCloud> PcdManager::merge_poindcloud(vector<std::shared_ptr<PointCloud>> pcds) {
	if (pcds.size() == 0) {
		return nullptr;
	}
	else if (pcds.size() == 1) {
		return pcds[0];
	}
	auto start = Now_ms();
	std::shared_ptr<PointCloud> pcd_(new PointCloud());
	// merge
	for (int i = 0; i < pcds.size(); i++) {
		pcd_->points_num += pcds[i]->points_num;
	}
	pcd_->position.resize(3, pcd_->points_num);
	pcd_->colors.resize(4, pcd_->points_num);
	pcd_->labels.resize(1, pcd_->points_num);
	pcd_->intensity.resize(1, pcd_->points_num);

	uint startCol = 0;
	for (int i = 0; i < pcds.size(); i++) {
		pcd_->position.block(0, startCol, 3, pcds[i]->points_num) = pcds[i]->position;
		pcd_->colors.block(0, startCol, 4, pcds[i]->points_num) = pcds[i]->colors;
		pcd_->labels.block(0, startCol, 1, pcds[i]->points_num) = pcds[i]->labels;
		pcd_->intensity.block(0, startCol, 1, pcds[i]->points_num ) = pcds[i]->intensity;
		startCol += pcds[i]->points_num;
	}
	pcd_->name = pcds[0]->name + "-merge";
	// BoundingBox
	Eigen::Vector3f minp_ = pcd_->position.rowwise().minCoeff();
	Eigen::Vector3f maxp_ = pcd_->position.rowwise().maxCoeff();
	pcd_->boundingBox.extend(minp_);
	pcd_->boundingBox.extend(maxp_);

	// Max Min Idensity
	pcd_->setmaxIdensity(pcd_->intensity.rowwise().maxCoeff()(0, 0));
	pcd_->setminIdensity(pcd_->intensity.rowwise().minCoeff()(0, 0));
	// Index Table
	pcd_->indexTable = new uint32_t[pcd_->points_num];
	// offset
	pcd_->offset = pcds[0]->offset;

	auto end = Now_ms();
	qDebug() << "Merge: " << end - start << "ms";

	return pcd_;
}