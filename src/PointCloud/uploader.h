/*
 * @Descripttion: 上载数据到gpu
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-01 14:45:06
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-01 14:45:20
 */
#pragma once

#include <iostream>
#include <vector>
#include <thread>

#include <QOpenGLWidget>
#include <QOpenGlFunctions_4_5_Core>

#include <Base/utils.h>
#include <Base/shader.h>
#include <PointCloud/pointclouds.h>

using namespace std;

class UpLoader : protected QOpenGLFunctions_4_5_Core {
public:
	std::shared_ptr<PointCloud> pcd = nullptr; // 读取的点云
	uint64_t prime = 0; //质数 

	vector<GLuint> vertexBuffers; //顶点缓冲区集合
	int maxPointsPerBuffer = 100000000; // 每个缓冲区最大数量点
	int bytesPerBuffer = 20; // 3 * float(4 bytes) + 4 * int_8(1 byte)
	GLuint indexBuffer; // 索引存储缓冲

	int defaultChunkSize = 500000; //上传的一个chunk的大小(点数量)
	GLuint Chunk16B; // 单个chunk缓冲区
	GLuint Chunk4B; // 单个chunk用于上传属性信息
	Shader *csShader; // 计算着色器 分发点信息 xyzrgba
	Shader *csShader_Attribute; // 计算着色器 分发属性 
	 
	int chunkIndex = 0; // 已上传的chunk索引

	int pointsUploaded = 0; //已上传的点


public:
	UpLoader(std::shared_ptr<PointCloud> &pcd); // 构造函数
	~UpLoader();

	int uploadNextChunk(); //上传点云数据至GPU 
	Eigen::MatrixXf getNextChunk(); // 获取下一个chunk的点(xyzrgb)
	int uploadData();
	Eigen::MatrixXf getData(); // 获取点云待上载数据
	bool isDone();

	int uploadChunkAttribute(void *data, int offset, int size); // 上传点属性至GPU
};