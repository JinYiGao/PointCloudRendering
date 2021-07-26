/*
 * @Descripttion: 多点云管理
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:27:45
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-21 14:28:04
 */
#pragma once

#include <iostream>
#include <vector>
#include <PointCloud/pointclouds.h>

using std::vector;

class PcdManager{
public:
	// 单例模式 全局唯一点云管理
	static PcdManager* GetInstance() {
		if (!m_pInstance)
			m_pInstance = new PcdManager();
		return m_pInstance;
	}

private:
	PcdManager();
	static PcdManager *m_pInstance;

protected:
	std::map<std::string, PointCloud *> pcdMap;

public:
	~PcdManager();

	bool add_pointcloud(PointCloud *pcd); // 添加进点云管理

	bool remove_pointcloud(PointCloud *pcd); // 移除点云管理
	bool remove_pointcloud(std::string name); 

	PointCloud* getPointCloud(std::string name); // 获取指定点云
	bool merge_poindcloud(vector<PointCloud *> pcds); // 合并若干点云
};