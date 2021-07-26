/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-05 12:02:27
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-08 23:22:07
 */
#include <framebuffer.h> 

// 构造函数 初始化  
FrameBuffer::FrameBuffer() {
	initializeOpenGLFunctions();

	this->depth = NULL;
	this->width = 0;
	this->height = 0;
	this->samples = 1;
	this->numColorAttachments = 1;

	glCreateFramebuffers(1, &fbo);

	setSize(512, 512);
}

void FrameBuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FrameBuffer::setNumColorAttachments(int numColorAttachments) {
	if (numColorAttachments == this->numColorAttachments) {
		return;
	}
	this->numColorAttachments = numColorAttachments;
	updateBuffers();
}

void FrameBuffer::setSamples(int samples) {
	if (samples == this->samples) {
		return;
	}
	this->samples = samples;
	updateBuffers();
}

void FrameBuffer::setSize(float width, float height) {
	if (this->width == width && this->height == height) {
		return;
	}
	this->width = width;
	this->height = height;
	updateBuffers();
}

void FrameBuffer::updateBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	float width = this->width;
	float height = this->height;
	int samples = this->samples;

	// 判断是否为多重采样纹理
	auto textureType = samples == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;

	auto oldTextures = this->textures;
	auto olddepth = this->depth;

	// 更新颜色纹理附件
	this->textures = {};
	for (int i = 0; i < this->numColorAttachments; i++) {
		GLuint texture;
		glGenTextures(1, &texture); // 创建纹理
		glBindTexture(textureType, texture); // 纹理绑定到目标
		if (samples == 1) {
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(textureType, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		else
		{
			glTexImage2DMultisample(textureType, samples, GL_RGBA8, width, height, false);
		}

		//附加纹理到帧缓冲
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textureType, texture, 0);
		this->textures.emplace_back(texture);
	}

	// 更新深度缓冲纹理附件
	glGenTextures(1, &depth);
	glBindTexture(textureType, depth);
	if (samples == 1) {
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(textureType, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	else
	{
		glTexImage2DMultisample(textureType, samples, GL_DEPTH_COMPONENT, width, height, false);
	}

	// 附加深度缓冲纹理到帧缓冲
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureType, depth, 0);

	// 清除旧的纹理
	for(int i = 0; i < oldTextures.size(); i++){
		glDeleteTextures(1, &oldTextures[i]);
	}
	if(olddepth){
		glDeleteTextures(1, &olddepth);
	}

	glBindTexture(textureType, 0);
}