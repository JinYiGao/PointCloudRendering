/*
 * @Descripttion: FPS风格相机类
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-04 13:56:02
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-04 13:56:18
 */

#include <Base/camera.h>
#include <iostream>
#include <QTime>

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
Eigen::Matrix4f Camera::lookAt(const Eigen::Vector3f &origin, const Eigen::Vector3f &target, const Eigen::Vector3f up)
{
	Eigen::Vector3f f = (target - origin).normalized();
	Eigen::Vector3f s = f.cross(up).normalized();
	Eigen::Vector3f u = s.cross(f);

	Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
	result(0, 0) = s(0);
	result(0, 1) = s(1);
	result(0, 2) = s(2);
	result(1, 0) = u(0);
	result(1, 1) = u(1);
	result(1, 2) = u(2);
	result(2, 0) = -f(0);
	result(2, 1) = -f(1);
	result(2, 2) = -f(2);
	result(0, 3) = -s.transpose() * origin;
	result(1, 3) = -u.transpose() * origin;
	result(2, 3) = f.transpose() * origin;
	//std::cout << result << std::endl;
	return result;
}

Eigen::Matrix4f Camera::ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
{
	Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
	result(0, 0) = 2.0f / (right - left);
	result(1, 1) = 2.0f / (top - bottom);
	result(2, 2) = -2.0f / (farVal - nearVal);
	result(0, 3) = -(right + left) / (right - left);
	result(1, 3) = -(top + bottom) / (top - bottom);
	result(2, 3) = -(farVal + nearVal) / (farVal - nearVal);
	return result;
}

Eigen::Matrix4f Camera::frustum(float left, float right, float bottom, float top, float nearVal, float farVal) {
	Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
	result(0, 0) = (2.0f * nearVal) / (right - left);
	result(1, 1) = (2.0f * nearVal) / (top - bottom);
	result(0, 2) = (right + left) / (right - left);
	result(1, 2) = (top + bottom) / (top - bottom);
	result(2, 2) = -(farVal + nearVal) / (farVal - nearVal);
	result(3, 2) = -1.0f;
	result(2, 3) = -(2.0f * farVal * nearVal) / (farVal - nearVal);
	return result;
}

//获得缩放矩阵
Eigen::Matrix4f scale(const Eigen::Vector3f &v) 
{ 
	return Eigen::Affine3f(Eigen::Scaling(v)).matrix(); 
}

//获得平移矩阵
Eigen::Matrix4f translate(const Eigen::Vector3f &v) 
{
	return Eigen::Affine3f(Eigen::Translation<float, 3>(v)).matrix();
}

//创建模型矩阵  ---包含模型 旋转 平移 缩放
Eigen::Matrix4f Camera::createModel(Eigen::Matrix4f rotation, Eigen::Vector3f translation, float zoom)
{
	//先旋转 后平移
	Eigen::Matrix4f model = translate(translation) * rotation * scale(Eigen::Vector3f::Constant(zoom));
	return model;
}

std::tuple<Eigen::Matrix4f, Eigen::Matrix4f, Eigen::Matrix4f> Camera::get_mvp()
{
	Eigen::Matrix4f model, view, proj;
	view = lookAt(position, target, up);//创建观察矩阵
	
	float fH = std::tan(view_angle / 360.0f * M_PI) * position.z();
	float fW = fH * (float)size.x() / (float)size.y();
	if (is_ortho)
	{
		proj = ortho(-fW, fW, -fH, fH, znear, zfar);
	}
	else 
	{
		proj = frustum(-fW, fW, -fH, fH, znear, zfar);
	}

	Eigen::Matrix4f rotation = arcball.matrix();
	//std::cout << arcball.state().matrix() << std::endl;
	view = view;
	//模型矩阵理论上应该包含物体最初的 平移 旋转 缩放 状态
	//控制相机视角下 模型的状态 不影响实际坐标 实际模型坐标由另一个模型矩阵来进行控制
	model = createModel(rotation, model_translation, zoom);
	//std::cout << model.matrix() << std::endl;
	return std::make_tuple(model, view, proj);
}

Eigen::Matrix4f Camera::getTransform()
{
	std::tuple<Eigen::Matrix4f, Eigen::Matrix4f, Eigen::Matrix4f> mvp = get_mvp();
	Eigen::Matrix4f model = std::get<0>(mvp);
	Eigen::Matrix4f view = std::get<1>(mvp);
	Eigen::Matrix4f proj = std::get<2>(mvp);
	
	return  proj * view * model;
}

void Camera::start_translate(const Vector2f &screen_point)
{
	M_start_translate = screen_point;
	is_translate = true;
}

void Camera::motion_translate(const Vector2f &screen_point)
{
	if (is_translate)
	{
		Vector2f translate = screen_point - M_start_translate;
		Vector3f tran = { translate.x(),-translate.y(),0.0f };
		tran *= sensitivity;
		//通过变换相机位置 达到上下左右平移的效果
		//Eigen::Vector3f targetVec = target - position; //环绕移动

		//Eigen::Vector3f rightVec = target.cross(up).normalized();
		//position += rightVec * tran.x();//左右移动
		//position += up * tran.y();//上下移动
		//target = up.cross(rightVec);

		// 平移物体
		model_translation += tran;
		M_start_translate = screen_point;
	}
}

void Camera::end_translate(const Vector2i &screen_point)
{
	is_translate = false;
}

void Camera::start_rotate(const Vector2i &screen_point) {
	arcball.button(screen_point.cast<int>(), true);
	is_rotate = true;
}

void Camera::motion_rotate(const Vector2i &screen_point) {
	if (!is_rotate)
		return;
	arcball.motion(screen_point.cast<int>());
}

void Camera::end_rotate(const Vector2i &screen_point) {
	motion_rotate(screen_point);

	arcball.button(screen_point, false);
	is_rotate = false;
}
