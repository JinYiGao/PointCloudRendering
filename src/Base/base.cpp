/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-03-03 21:13:09
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-03-03 21:27:02
 */

#include <base.h>

unsigned char* Base::load_img(const QString &filename, int &width, int &height,int &Format)
{
	QImage *img=new QImage(filename);
	width = img->width();
	height = img->height(); 

	QImage::Format ToFormat;
	Format = img->format();
	switch (Format)
	{
		case QImage::Format_RGB32:
			ToFormat = QImage::Format_RGB888;
			break;
		case QImage::Format_ARGB32:
			ToFormat = QImage::Format_RGBA8888;
			break;
		default:
			break;
	}
	Format = ToFormat;
	*img = img->convertToFormat(ToFormat);

	return img->bits();
}