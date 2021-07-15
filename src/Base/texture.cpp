/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-09 21:35:46
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-09 21:35:46
 */
#include <Base/texture.h>

Texture::Texture() {

}
// 创建指定大小空纹理
Texture::Texture(GLint width, GLint height, GLint internalformat) {
	initializeOpenGLFunctions();
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	this->type = GL_TEXTURE_2D;
	this->width = width;
	this->height = height;
}

// 根据图片创建纹理
Texture::Texture(const char *filename)
{ 
	initializeOpenGLFunctions();

	int width, height, format;
	unsigned char *data = Base::load_img(filename, width, height, format);
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (format == QImage::Format_RGB888)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else if (format == QImage::Format_RGBA8888)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glGenerateMipmap(GL_TEXTURE_2D); //自动生成多层渐进纹理

	glBindTexture(GL_TEXTURE_2D, 0);

	this->type = GL_TEXTURE_2D;
	this->width = width;
	this->height = height;
}

// 创建立方体贴图
GLuint Texture::createCubeMap(std::vector<std::string> faces) {
	initializeOpenGLFunctions();
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	//创建每一个面的纹理
	int width, height, format;
	for (int i = 0; i < faces.size(); i++)
	{
		//读取数据
		unsigned char *data = Base::load_img(QString::fromStdString(faces[i]), width, height, format);
		if (format == QImage::Format_RGB888)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (format == QImage::Format_RGBA8888)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	this->type = GL_TEXTURE_CUBE_MAP;
	this->width = width;
	this->height = height;

	return ID;
}

void Texture::bind() {
	glBindTexture(this->type, this->ID);
}

void Texture::bind(uint index) {
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(this->type, this->ID);
}

void Texture::release() {
	glBindTexture(this->type, 0);
}

void Texture::setTexParameteri(GLenum pname, GLint param) {
	bind();
	glTexParameteri(this->type, pname, param);
	release();
}

void Texture::destroy() {
	glDeleteTextures(1, &ID);
}