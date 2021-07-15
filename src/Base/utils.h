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
#include <chrono>
#include <ctime>

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


// 根据OpenGL Type编码获取类型占字节
static size_t TypeSize(GLenum type)
{
	size_t size;

#define CASE(Enum, Count, Type) \
case Enum: size = Count * sizeof(Type); break

	switch (type) {
		CASE(GL_FLOAT, 1, GLfloat);
		CASE(GL_FLOAT_VEC2, 2, GLfloat);
		CASE(GL_FLOAT_VEC3, 3, GLfloat);
		CASE(GL_FLOAT_VEC4, 4, GLfloat);
		CASE(GL_INT, 1, GLint);
		CASE(GL_INT_VEC2, 2, GLint);
		CASE(GL_INT_VEC3, 3, GLint);
		CASE(GL_INT_VEC4, 4, GLint);
		CASE(GL_UNSIGNED_INT, 1, GLuint);
		CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
		CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
		CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
		CASE(GL_BOOL, 1, GLboolean);
		CASE(GL_BOOL_VEC2, 2, GLboolean);
		CASE(GL_BOOL_VEC3, 3, GLboolean);
		CASE(GL_BOOL_VEC4, 4, GLboolean);
		CASE(GL_FLOAT_MAT2, 4, GLfloat);
		CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
		CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
		CASE(GL_FLOAT_MAT3, 9, GLfloat);
		CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
		CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
		CASE(GL_FLOAT_MAT4, 16, GLfloat);
		CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
		CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
#undef CASE
	default:
		fprintf(stderr, "Unknown type: 0x%x\n", type);
		exit(EXIT_FAILURE);
		break;
	}

	return size;
}