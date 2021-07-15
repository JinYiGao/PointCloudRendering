/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-03 21:05:48
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-03 21:13:00
 */
#pragma once

#include <QImage>
#include <iostream>

class Base
{ 
public: 
	//采用QImage方式读取OpenGL纹理图像
	static unsigned char* load_img(const QString &filename,int &width,int &height,int &Format);
};