/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-07-05 12:02:27
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-07-08 23:22:07
 */
#include <framebuffer.h>
#include <qDebug>

static bool color_format_compatible(GLenum internal_format, GLenum format, GLenum type)
{
	if (format != GL_RED && format != GL_RG && format != GL_RGB && format != GL_BGR && format != GL_RGBA && format != GL_BGRA) {
		qDebug() << "the provided format is not accepted";
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	if (type != GL_UNSIGNED_BYTE && type != GL_BYTE && type != GL_UNSIGNED_SHORT && type != GL_SHORT && type != GL_UNSIGNED_INT && type != GL_INT &&
		type != GL_FLOAT && type != GL_UNSIGNED_BYTE_3_3_2 && type != GL_UNSIGNED_BYTE_2_3_3_REV && type != GL_UNSIGNED_SHORT_5_6_5 &&
		type != GL_UNSIGNED_SHORT_5_6_5_REV && type != GL_UNSIGNED_SHORT_4_4_4_4 && type != GL_UNSIGNED_SHORT_4_4_4_4_REV &&
		type != GL_UNSIGNED_SHORT_5_5_5_1 && type != GL_UNSIGNED_SHORT_1_5_5_5_REV && type != GL_UNSIGNED_INT_8_8_8_8 &&
		type != GL_UNSIGNED_INT_8_8_8_8_REV && type != GL_UNSIGNED_INT_10_10_10_2 && type != GL_UNSIGNED_INT_2_10_10_10_REV)
	{
		qDebug() << "the provided type is not accepted";
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	if (internal_format != GL_R8 && internal_format != GL_R8I && internal_format != GL_R8UI &&
		internal_format != GL_R16 && internal_format != GL_R16I && internal_format != GL_R16UI && internal_format != GL_R16F &&
		internal_format != GL_R32I && internal_format != GL_R32UI && internal_format != GL_R32F &&
		internal_format != GL_RG8 && internal_format != GL_RG8I && internal_format != GL_RG8UI &&
		internal_format != GL_RG16 && internal_format != GL_RG16I && internal_format != GL_RG16UI && internal_format != GL_RG16F &&
		internal_format != GL_RG32I && internal_format != GL_RG32UI && internal_format != GL_RG32F &&
		internal_format != GL_RGB8 && internal_format != GL_RGB8I && internal_format != GL_RGB8UI &&
		internal_format != GL_RGB16 && internal_format != GL_RGB16I && internal_format != GL_RGB16UI && internal_format != GL_RGB16F &&
		internal_format != GL_RGB32I && internal_format != GL_RGB32UI && internal_format != GL_RGB32F &&
		internal_format != GL_RGBA8 && internal_format != GL_RGBA8I && internal_format != GL_RGBA8UI &&
		internal_format != GL_RGBA16 && internal_format != GL_RGBA16I && internal_format != GL_RGBA16UI && internal_format != GL_RGBA16F &&
		internal_format != GL_RGBA32I && internal_format != GL_RGBA32UI && internal_format != GL_RGBA32F
		)
	{
		qDebug() << "internal format must be in GL_[components][size][type] format";
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	if (internal_format == GL_R8 || internal_format == GL_R8I || internal_format == GL_R8UI ||
		internal_format == GL_R16 || internal_format == GL_R16I || internal_format == GL_R16UI || internal_format == GL_R16F ||
		internal_format == GL_R32I || internal_format == GL_R32UI || internal_format == GL_R32F
		)
	{
		if (format != GL_RED) {
			qDebug() << "color format must be GL_R";
			return false;
		}
	}
	else if (internal_format == GL_RG8 || internal_format == GL_RG8I || internal_format == GL_RG8UI ||
		internal_format == GL_RG16 || internal_format == GL_RG16I || internal_format == GL_RG16UI || internal_format == GL_RG16F ||
		internal_format == GL_RG32I || internal_format == GL_RG32UI || internal_format == GL_RG32F
		)
	{
		if (format != GL_RG) {
			qDebug() << "color format must be GL_RG";
			return false;
		}
	}
	else if (internal_format == GL_RGB8 || internal_format == GL_RGB8I || internal_format == GL_RGB8UI ||
		internal_format == GL_RGB16 || internal_format == GL_RGB16I || internal_format == GL_RGB16UI || internal_format == GL_RGB16F ||
		internal_format == GL_RGB32I || internal_format == GL_RGB32UI || internal_format == GL_RGB32F
		)
	{
		if (format != GL_RGB) {
			qDebug() << "color format must be GL_RGB";
			return false;
		}
	}
	else if (internal_format == GL_RGBA8 || internal_format == GL_RGBA8I || internal_format == GL_RGBA8UI ||
		internal_format == GL_RGBA16 || internal_format == GL_RGBA16I || internal_format == GL_RGBA16UI || internal_format == GL_RGBA16F ||
		internal_format == GL_RGBA32I || internal_format == GL_RGBA32UI || internal_format == GL_RGBA32F
		)
	{
		if (format != GL_RGBA) {
			qDebug() << "color format must be GL_RGBA";
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	// TODO: check signed, unsigned, byte, int, float compatibility...

	return true;
}

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

FrameBuffer::~FrameBuffer() {

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


// 添加颜色纹理附件
bool FrameBuffer::add_color_texture(
	GLenum internal_format,
	GLenum format,
	GLenum type,
	GLenum filter) 
{
	if (fbo == 0) {
		qDebug() << "fbo not created!";
		return false;
	}

	if (!color_format_compatible(internal_format, format, type)) {
		qDebug() << "Attaching color texture failed!";
		return false;
	}

	GLint max_attachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);
	if (colorAttachments.size() >= max_attachments) {
		qDebug() << "Maxmum color attachment reached!";
		return false;
	}

	// 当前添加的纹理附件index
	int index = colorAttachments.size();
	ColorAttachment attachment;
	attachment.internal_format = internal_format;
	attachment.format = format;
	attachment.type = type;
	attachment.texture_filter = filter;

	// Create Color Texture
	glGenTextures(1, &(attachment.texture));
	
	GLenum texture_target;
	// 多重采样纹理
	if (samples > 0) {
		texture_target = GL_TEXTURE_2D_MULTISAMPLE;
		glBindTexture(texture_target, attachment.texture);
		glTexImage2DMultisample(texture_target, samples, internal_format, width, height, GL_TRUE);
	}
	else {
		texture_target = GL_TEXTURE_2D;
		glBindTexture(texture_target, attachment.texture);

		glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(texture_target, 0, internal_format, width, height, 0, format, type, nullptr);
	}

}
