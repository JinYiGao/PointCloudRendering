/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-01 14:45:27
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-01 14:45:28
 */
#include <uploader.h>

uint64_t previousPrimeCongruent3mod4(uint64_t start);

// 构造函数
UpLoader::UpLoader(PointCloud *pcd) {
	this->pcd = pcd;

	// 初始化
	initializeOpenGLFunctions();

	prime = previousPrimeCongruent3mod4(pcd->points_num); // 根据点云数量生成一个最大质数
	
	// 点云数量太大时 创建多个顶点缓冲区
	int numBuffers = pcd->points_num / maxPointsPerBuffer + 1; // 需要的vertexBuffer数量
	if (pcd->points_num % maxPointsPerBuffer == 0) {
		numBuffers -= 1;
	}

	// 为所有点开辟空缓冲区 用于计算着色器计算后填入相应点信息
	GLbitfield usage = GL_STATIC_DRAW;
	uint64_t pointsLeft = pcd->points_num;
	for (int i = 0; i < numBuffers; i++) {
		int numPointsInBuffer = pointsLeft > maxPointsPerBuffer ? maxPointsPerBuffer : pointsLeft;
		GLuint vertexBuffer;
		glCreateBuffers(1, &vertexBuffer);

		uint32_t size = numPointsInBuffer * bytesPerBuffer; 
		glNamedBufferData(vertexBuffer, size, nullptr, usage);
		vertexBuffers.emplace_back(vertexBuffer);
		pointsLeft -= numPointsInBuffer;
	}

	// 为单个chunk开辟缓冲区 用于分批填入点云xyzrgba数据 上传数据进行位置分配计算
	uint32_t chunkSize = defaultChunkSize * bytesPerBuffer;
	glCreateBuffers(1, &Chunk16B);
	glNamedBufferData(Chunk16B, chunkSize, nullptr, usage);

	glCreateBuffers(1, &Chunk4B);
	glNamedBufferData(Chunk4B, chunkSize, nullptr, usage);

	// 创建计算着色器
	csShader = new Shader(":/PointCloud/shaders/distribute.cs");
	csShader_Attribute = new Shader(":/PointCloud/shaders/distribute_attribute.cs");

	// 上载点数据至gpu
	int64_t start = Now_ms();
	//while (!isDone()) {
	//	uploadNextChunk();
	//	//std::cout << "pointsUploaded: " << pointsUploaded << std::endl;
	//}
	uploadData();
	int64_t end = Now_ms();
	std::cout << "Time of UpLoad To GPU: " << end - start << "ms" << std::endl;
}

// 上载数据 一次上传单个chunk
int UpLoader::uploadNextChunk() {
	// 获取一个chunk的点 和 颜色信息
	Eigen::MatrixXf xyzrgba = getNextChunk();
	if (xyzrgba.rows() <= 0) {
		return 0;
	}

	int chunkSize = xyzrgba.cols(); // cols = num

	// upload
	glNamedBufferSubData(Chunk16B, 0, chunkSize * bytesPerBuffer, xyzrgba.data());

	// 利用计算着色器更改点位置
	csShader->bind(); // 绑定着色器

	GLuint Input = Chunk16B;
	// 指定缓冲目标 并绑定到着色器内绑定点 一一对应
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Input);
	for (int i = 0; i < vertexBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + i, vertexBuffers[i]);
	}
	// 传入uniform变量值
	csShader->setUniformValue("MaxPointsPerBuffer", maxPointsPerBuffer);
	csShader->setUniformValue("uNumPoints", chunkSize); // 该次传入着色器的点数量
	int uPrime = int(prime);
	csShader->setUniformValue("uPrime", uPrime);
	csShader->setUniformValue("uOffset", pointsUploaded); // 已上传的点

	// 计算全局工作组大小
	//  --- 一个本地工作组的一个执行单元对应计算一个点
	int groups = int(ceil(double(chunkSize) / 32.0)); // 向上取整
	glDispatchCompute(groups, 1, 1);

	// unbind buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	for (int i = 0; i < vertexBuffers.size(); i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + i, 0);
	}
	csShader->release(); // 解绑着色器
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	pointsUploaded += xyzrgba.cols();

	return xyzrgba.cols();
}

// 获取一个chunk的点信息(xyzrgba)
Eigen::MatrixXf UpLoader::getNextChunk() {
	Eigen::MatrixXf xyz;
	Eigen::MatrixXi_8 rgba;
	Eigen::MatrixXf xyzrgba;
	int startCol = defaultChunkSize * chunkIndex >= pcd->position.cols() ? -1 : defaultChunkSize * chunkIndex;
	if (startCol == -1) {
		return xyzrgba;
	}
	int blockCol = pcd->position.cols() - defaultChunkSize * chunkIndex > defaultChunkSize ? defaultChunkSize : pcd->position.cols() - defaultChunkSize * chunkIndex;
	xyz.resize(3, blockCol);
	xyz = pcd->position.block(0, startCol, 3, blockCol);
	rgba.resize(4, blockCol);
	rgba = pcd->colors.block(0, startCol, 4, blockCol);
	//uint8转为float
	Eigen::MatrixXf i2f;
	i2f.resize(1, blockCol);
	memcpy(i2f.data(), rgba.data(), 4 * blockCol * sizeof(uint8_t)); // 内存拷贝
	xyzrgba.resize(4, blockCol);
	xyzrgba << xyz,
		i2f;

	chunkIndex++;

	return xyzrgba;
}

int UpLoader::uploadData() {
	Eigen::MatrixXf xyzrgba = getData();
	if (xyzrgba.rows() <= 0) {
		return 0;
	}
	int Size = xyzrgba.cols(); // 上传数据个数
	int num = Size / defaultChunkSize + 1; // 需要分批上传次数
	if (num % defaultChunkSize == 0) {
		num -= 1;
	}
	int leftSize = Size;
	void* data = xyzrgba.data();
	for (int i = 0; i < num; i++) {
		int chunkSize = leftSize > defaultChunkSize ? defaultChunkSize : leftSize;
		// 上传数据装载
		glNamedBufferSubData(Chunk16B, 0, chunkSize * bytesPerBuffer, data);
		leftSize -= chunkSize;
		data = (void *)((uint8_t*)data + chunkSize * bytesPerBuffer);

		// 绑定着色器
		csShader->bind();
		GLuint input = Chunk16B;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
		for (int j = 0; j < vertexBuffers.size(); j++) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, j + 2, vertexBuffers[j]);
		}
		csShader->setUniformValue("MaxPointsPerBuffer", maxPointsPerBuffer);
		csShader->setUniformValue("uNumPoints", chunkSize);
		int uPrime = prime;
		csShader->setUniformValue("uPrime", uPrime);
		csShader->setUniformValue("uOffset", pointsUploaded);

		// 计算全局工作组大小
		//  --- 一个本地工作组的一个执行单元对应计算一个点
		int groups = int(ceil(double(chunkSize) / 32.0)); // 向上取整
		glDispatchCompute(groups, 1, 1);

		// unbind buffer
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		for (int j = 0; j < vertexBuffers.size(); j++) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + j, 0);
		}
		csShader->release(); // 解绑着色器
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		pointsUploaded += chunkSize;
		//std::cout <<"PointsUploaded: " << pointsUploaded << std::endl;
	}
	return Size;
}

// 获取点云待上载数据
Eigen::MatrixXf UpLoader::getData() {
	Eigen::MatrixXf xyz;
	Eigen::MatrixXi_8 rgba;
	Eigen::MatrixXf xyzrgba;
	Eigen::MatrixXi show;

	xyz = pcd->position;
	rgba = pcd->colors;
	//uint8转为float
	Eigen::MatrixXf i2f;
	uint32_t cols = pcd->points_num;
	i2f.resize(1, cols);
	memcpy(i2f.data(), rgba.data(), 4 * cols); // 内存拷贝

	Eigen::MatrixXf i4B2f;
	show.resize(1, cols);
	show = Eigen::MatrixXi::Ones(1, cols);
	i4B2f.resize(1, cols);
	memcpy(i4B2f.data(), show.data(), 4 * cols); // 内存拷贝
	xyzrgba.resize(5, cols);
	xyzrgba << xyz,
		i2f,
		i4B2f;
	return xyzrgba;
}

bool UpLoader::isDone() {
	if (defaultChunkSize * chunkIndex >= pcd->position.cols()) {
		return true;
	}
	return false;
}

// 上载更新属性信息 -- 单个点 只能4字节
int UpLoader::uploadChunkAttribute(void *data, int offset, int size) {
	if (data == nullptr) {
		return 0;
	}
	int targetOffset = offset; // 上传属性的起始偏移
	int Size = size; // 上传数据个数
	int num = size / defaultChunkSize; // 需要分批上传次数
	if (num % defaultChunkSize == 0) {
		num -= 1;
	}
	int leftSize = Size;
	for (int i = 0; i < num; i++) {
		targetOffset = offset + i * defaultChunkSize;
		int chunkSize = leftSize > defaultChunkSize ? defaultChunkSize : leftSize;
		// 上传数据装载
		glNamedBufferSubData(Chunk4B, 0, chunkSize * 4, data);
		leftSize -= chunkSize;
		data = (void*)((uint*)data + chunkSize);
		
		// 绑定着色器
		csShader_Attribute->bind();
		GLuint input = Chunk4B;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
		for (int j = 0; j < vertexBuffers.size(); j++) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, j + 2, vertexBuffers[j]);
		}
		csShader_Attribute->setUniformValue("MaxPointsPerBuffer", maxPointsPerBuffer);
		csShader_Attribute->setUniformValue("uNumPoints", chunkSize);
		int uPrime = prime;
		csShader_Attribute->setUniformValue("uPrime", uPrime);
		csShader_Attribute->setUniformValue("uOffset", targetOffset);
		//std::cout << targetOffset << std::endl;

		// 计算全局工作组大小
		//  --- 一个本地工作组的一个执行单元对应计算一个点
		int groups = int(ceil(double(chunkSize) / 32.0)); // 向上取整
		glDispatchCompute(groups, 1, 1);

		// unbind buffer
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		for (int j = 0; j < vertexBuffers.size(); j++) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + j, 0);
		}
		csShader_Attribute->release(); // 解绑着色器
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	return size;
}

// 判断是否为质数
bool isPrime(uint64_t n) {
	if (n <= 3) {
		return n > 1;
	}
	else if ((n % 2) == 0 || (n % 3) == 0) {
		return false;
	}
	uint64_t i = 5;
	while ((i * i) <= n) {
		if ((n % i) == 0 || (n % (i + 2)) == 0) {
			return false;
		}
		i = i + 6;
	}

	return true;
}

//生成质数
// Primes where p = 3 mod 4 allow us to generate random numbers without duplicates in range [0, prime - 1]
uint64_t previousPrimeCongruent3mod4(uint64_t start) {
	for (uint64_t i = start - 1; true; i--) {
		if ((i % 4) == 3 && isPrime(i)) {
			return i;
		}
	}
}