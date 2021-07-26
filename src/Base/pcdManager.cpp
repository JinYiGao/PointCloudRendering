/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:27:54
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-21 14:27:55
 */
#include <pcdManager.h>

PcdManager::PcdManager() {
	std::cout << "Single Manager" << std::endl;
}
PcdManager* PcdManager::m_pInstance = nullptr;

PcdManager::~PcdManager() {
}

bool PcdManager::add_pointcloud(PointCloud *pcd) {
	pcdMap.emplace(pcd->name, pcd);
	return true;
}

bool PcdManager::remove_pointcloud(PointCloud *pcd) {
	std::map<std::string, PointCloud *>::iterator iter =  pcdMap.find(pcd->name);
	if (iter != pcdMap.end()) {
		pcdMap.erase(iter);
		delete pcd;
		return true;
	}
	else {
		return false;
	}
}

bool PcdManager::remove_pointcloud(std::string name) {
	std::map<std::string, PointCloud *>::iterator iter = pcdMap.find(name);
	if (iter != pcdMap.end()) {
		auto pcd = pcdMap.at(name);
		pcdMap.erase(iter);
		delete pcd;
		return true;
	}
	else {
		return false;
	}
}

PointCloud* PcdManager::getPointCloud(std::string name) {
	std::map<std::string, PointCloud *>::iterator iter = pcdMap.find(name);
	if (iter != pcdMap.end()) {
		auto pcd = pcdMap.at(name);
		return pcd;
	}
	else{
		return nullptr;
	}
}

bool PcdManager::merge_poindcloud(vector<PointCloud *>pcds) {
	return true;
}