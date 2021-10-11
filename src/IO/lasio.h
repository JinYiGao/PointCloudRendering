/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-01 13:48:36
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-31 23:35:47
 */
#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <QFileInfo>
#include <PointCloud/pointclouds.h>
#include <Base/utils.h>

using std::vector;

static Eigen::Vector3f offset_; // 所有点云有一个统一的偏移值

void read_las(std::string path, std::shared_ptr<PointCloud> &pcd);
void save_las(const std::string &path, vector<std::shared_ptr<PointCloud>> pcds);