/*
 * @Descripttion: 多点云管理
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-21 14:27:45
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:35:25
 */
#pragma once

#include <iostream>
#include <vector>
#include <QString>
#include <QDebug>
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
	std::map<QString, std::shared_ptr<PointCloud>> pcdMap;

public:
	~PcdManager();

	bool addPcdToManager(std::shared_ptr<PointCloud> &pcd); // 添加进点云管理

	bool remove_pointcloud(std::shared_ptr<PointCloud> &pcd); // 移除点云管理
	bool remove_pointcloud(QString name);

	bool existPcd(QString name); // 判断点云是否存在
	std::shared_ptr<PointCloud> getPointCloud(QString name); // 获取指定点云
	std::shared_ptr<PointCloud> merge_poindcloud(vector<std::shared_ptr<PointCloud>> pcds); // 合并若干点云
};