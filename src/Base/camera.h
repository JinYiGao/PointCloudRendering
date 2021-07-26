/*
 * @Descripttion: FPS风格相机类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-04 13:55:56
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-04 13:56:25
 */
#pragma once
#include <Eigen/Eigen>
#include <tuple>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif 

using namespace Eigen;
/**
 * \struct Arcball glutil.h nanogui/glutil.h
 *
 * \brief Arcball helper class to interactively rotate objects on-screen.
 */
struct Arcball {
	Arcball(float speedFactor = 2.0f)
		: mActive(false), mLastPos(Vector2i::Zero()), mSize(Vector2i::Zero()), mQuat(Quaternionf::Identity()),
		mIncr(Quaternionf::Identity()), mSpeedFactor(speedFactor) {}

	Arcball(const Quaternionf &quat)
		: mActive(false), mLastPos(Vector2i::Zero()), mSize(Vector2i::Zero()), mQuat(quat),
		mIncr(Quaternionf::Identity()), mSpeedFactor(2.0f) {}

	Quaternionf &state() { return mQuat; }

	void setState(const Quaternionf &state) {
		mActive = false;
		mLastPos = Vector2i::Zero();
		mQuat = state;
		mIncr = Quaternionf::Identity();
	}

	void setSize(Vector2i size) { mSize = size; }
	const Vector2i &size() const { return mSize; }
	void setSpeedFactor(float speedFactor) { mSpeedFactor = speedFactor; }
	float speedFactor() const { return mSpeedFactor; }
	bool active() const { return mActive; }

	void button(Vector2i pos, bool pressed) {
		mActive = pressed;
		mLastPos = pos;
		if (!mActive)
			mQuat = (mIncr * mQuat).normalized();
		mIncr = Quaternionf::Identity();
	}

	bool motion(Vector2i pos) {
		if (!mActive)
			return false;

		/* Based on the rotation controller form AntTweakBar */
		float invMinDim = 1.0f / mSize.minCoeff();
		float w = (float)mSize.x(), h = (float)mSize.y();

		float ox = (mSpeedFactor * (2 * mLastPos.x() - w) + w) - w - 1.0f;
		float tx = (mSpeedFactor * (2 * pos.x() - w) + w) - w - 1.0f;
		float oy = (mSpeedFactor * (h - 2 * mLastPos.y()) + h) - h - 1.0f;
		float ty = (mSpeedFactor * (h - 2 * pos.y()) + h) - h - 1.0f;

		ox *= invMinDim;
		oy *= invMinDim;
		tx *= invMinDim;
		ty *= invMinDim;

		Vector3f v0(ox, oy, 1.0f), v1(tx, ty, 1.0f);
		if (v0.squaredNorm() > 1e-4f && v1.squaredNorm() > 1e-4f) {
			v0.normalize();
			v1.normalize();
			Vector3f axis = v0.cross(v1);
			float sa = std::sqrt(axis.dot(axis)), ca = v0.dot(v1), angle = std::atan2(sa, ca);
			if (tx * tx + ty * ty > 1.0f)
				angle *= 1.0f + 0.2f * (std::sqrt(tx * tx + ty * ty) - 1.0f);
			mIncr = Eigen::AngleAxisf(angle, axis.normalized());
			if (!std::isfinite(mIncr.norm()))
				mIncr = Quaternionf::Identity();
		}
		return true;
	}

	Matrix4f matrix() const {
		Matrix4f result2 = Matrix4f::Identity();
		result2.block<3, 3>(0, 0) = (mIncr * mQuat).toRotationMatrix();
		return result2;
	}

protected:
	bool mActive;
	Vector2i mLastPos;
	Vector2i mSize;
	Quaternionf mQuat, mIncr;
	float mSpeedFactor;
};

class Camera
{
public:
	//Camera Attributes
	Eigen::Vector3f position = { 0.0f, 0.0f, 1.0f };//相机位置
	Eigen::Vector3f target = { 0.0f, 0.0f, -0.01f };//相机观察目标方向
	Eigen::Vector3f up = { 0.0f, 1.0f, 0.0f };//上向量

	//** 控制model矩阵 **
	// 旋转
	Arcball arcball;
	// 控制模型平移
	Eigen::Vector3f model_translation = Eigen::Vector3f::Zero();
	// 控制模型初始缩放
	float zoom = 1.0f;

	//控制projection矩阵
	float view_angle = 5.0f;//fov上下视野角度 设置初始视图
	float znear = 0.1f, zfar = 100.0f;//近截面 远截面

	//图形显示屏幕大小
	Eigen::Vector2i size = { 800, 600 };

	//是否采用正射投影
	bool is_ortho = false;

	//是否正在平移
	bool is_translate = false;

	//记录下平移前鼠标位置
	Vector2f M_start_translate = {0.0f,0.0f};
	float sensitivity = 0.002;

	//是否正在旋转
	bool is_rotate = false;
	

	//------------观察矩阵 (View)-------------
	//----------------------------------------
	//         Rx  Ry  Rz  0     1  0  0  -Px
	//         Ux  Uy  Uz  0     0  1  0  -Py
	// LookAt= Dx  Dy  Dz  0  *  0  0  1  -Pz
	//         0   0   0   1     0  0  0   1
	//----------------------------------------
	//* origin 相机初始位置 
	//* target 相机观察目标位置
	//* up 上向量 ，用于创建 相机坐标系
	//
	//利用该观察矩阵，可以将物体世界坐标转换为相机坐标系下坐标
	Eigen::Matrix4f lookAt(const Eigen::Vector3f &origin, const Eigen::Vector3f &target, const Eigen::Vector3f up);

	//-----------正射投影-------------
	//
	//--------------------------------
	Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float nearVal, float farVal);

	//-----------透视投影-------------
	//
	//--------------------------------
	Eigen::Matrix4f frustum(float left, float right, float bottom, float top, float nearVal, float farVal);

	//-------------------------------
	// 获取 model: 模型矩阵
	//      view : 相机观察矩阵
	//      project: 投影矩阵
	//-------------------------------
	std::tuple<Eigen::Matrix4f, Eigen::Matrix4f, Eigen::Matrix4f> get_mvp();

	///\创建模型矩阵
	Eigen::Matrix4f createModel(Eigen::Matrix4f rotation, Eigen::Vector3f translate, float zoom);

	///\brief 获得最后的转换矩阵
	Eigen::Matrix4f getTransform();

	///\brief 开始平移
	void start_translate(const Vector2f &screen_point);
	///\brief 正在平移动作
	void motion_translate(const Vector2f &screen_point);
	///\brief 结束平移
	void end_translate(const Vector2i &screen_point);

	///\brief 开始旋转
	void start_rotate(const Vector2i &screen_point);
	///\brief 正在旋转时移动
	void motion_rotate(const Vector2i &screen_point);
	///\brief 结束旋转
	void end_rotate(const Vector2i &screen_point);
};