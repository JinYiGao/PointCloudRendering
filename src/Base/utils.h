/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-07 21:54:55
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-07 21:54:56
 */
#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QDebug>
#include <chrono>
#include <ctime>
#include <Eigen/Eigen>

using namespace std::chrono;

static int Now_seconds() {
	system_clock::time_point time_point_now = system_clock::now(); // 获取当前时间点
	system_clock::duration duration_since_epoch = time_point_now.time_since_epoch(); // 从1970-01-01 00:00:00到当前时间点的时长
	time_t microseconds_since_epoch = duration_cast<microseconds>(duration_since_epoch).count(); // 将时长转换为微秒数
	time_t seconds_since_epoch = microseconds_since_epoch / 1000000; // 将时长转换为秒数
	std::tm current_time = *std::localtime(&seconds_since_epoch); // 获取当前时间（精确到秒）
	return current_time.tm_sec;
}

static int64_t Now_us() {
	system_clock::time_point time_point_now = system_clock::now(); // 获取当前时间点
	system_clock::duration duration_since_epoch = time_point_now.time_since_epoch(); // 从1970-01-01 00:00:00到当前时间点的时长
	time_t microseconds_since_epoch = duration_cast<microseconds>(duration_since_epoch).count(); // 将时长转换为微秒数
	return microseconds_since_epoch;
}

static int64_t Now_ms() {
	system_clock::time_point time_point_now = system_clock::now(); // 获取当前时间点
	system_clock::duration duration_since_epoch = time_point_now.time_since_epoch(); // 从1970-01-01 00:00:00到当前时间点的时长
	time_t microseconds_since_epoch = duration_cast<microseconds>(duration_since_epoch).count(); // 将时长转换为微秒数
	time_t tm_millisec = microseconds_since_epoch / 1000; // 毫秒数
	return tm_millisec;
}

static long long start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
static double now() {
	auto now = std::chrono::high_resolution_clock::now();
	long long nanosSinceStart = now.time_since_epoch().count() - start_time;

	double secondsSinceStart = double(nanosSinceStart) / 1'000'000'000;

	return secondsSinceStart;
}

template<typename T>
static Eigen::Matrix<T, -1, -1> mergeMatrixLeftRight(Eigen::Matrix<T, -1, -1> left, Eigen::Matrix<T, -1, -1> right) {
	Eigen::Matrix<T, -1, -1> result;
	result.resize(left.rows(), left.cols() + right.cols());
	if (left.rows() != right.rows()) {
		qDebug() << "Merge Failed! Rows Not Equal!";
		return result;
	}
	result << left, right;
	return result;
}

template<typename T>
static Eigen::Matrix<T, -1, -1> mergeMatrixUpDown(Eigen::Matrix<T, -1, -1> up, Eigen::Matrix<T, -1, -1> down) {
	Eigen::Matrix<T, -1, -1> result;
	result.resize(up.rows() + down.rows(), up.cols());
	if (up.cols() != down.cols()) {
		qDebug() << "Merge Failed! Cols Not Equal!"; 
		return result;
	}
	result << up,
		down;
	return result;
}