/*
 * @Descripttion: las点云读取
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-05-29 18:34:36
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-05-29 18:35:28
 */

#include <qglobal.h>
#include <lasio.h>
#include <laszip/laszip_api.h>

PointCloud read_las(string path) {
	int64_t start = Now_ms();
	PointCloud pcd;

	QFileInfo info(QString::fromStdString(path));
	pcd.name = info.fileName().toStdString();

	laszip_POINTER reader;
	laszip_create(&reader); 

	laszip_BOOL is_compressed; // 文件是否压缩
	laszip_open_reader(reader, path.c_str(), &is_compressed);

	// 存储las文件头信息结构体
	laszip_header *header;
	laszip_get_header_pointer(reader, &header);

	// number of points
	int points_num = header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records;

	// 存储las单个点信息结构体
	laszip_point *point;
	laszip_get_point_pointer(reader, &point);

	pcd.position.resize(3, points_num); // 位置矩阵
	pcd.labels.resize(1, points_num); // 分类信息
	pcd.intensity.resize(1, points_num); // 强度信息
	pcd.offset = Eigen::Vector3f(header->x_offset, header->y_offset, header->z_offset); // 头文件中包含的偏移

	// 某些格式下含有颜色信息
	bool hasColor = false;
	int format = header->point_data_format;
	if (format == 2 || format == 3 || format == 5) {
		pcd.colors.resize(4, points_num);
		hasColor = true;
	}
	else{
		pcd.colors.resize(4, points_num);
	}

	// 读取点位数据
	uint32_t index = 0;
	bool is8bit = false;
	bool manual_offset = false; // 当头文件不包含偏移时, 手动执行偏移
	bool first = true;
	while (index < points_num) {
		// 取出一个点
		laszip_read_point(reader);

		//若头文件不含偏移信息，则手动给定偏移,移至原点附近
		if (first && pcd.offset.norm() < 0.01) {
			manual_offset = true;
			pcd.offset.x() = point->X * header->x_scale_factor;
			pcd.offset.y() = point->Y * header->y_scale_factor;
			pcd.offset.z() = point->Z * header->z_scale_factor;
			qWarning("The header does not contain offset information, use the first point!\n");
		}
		first = false;

		// 读取坐标
		Eigen::Vector3f point3d;
		if (manual_offset) {
			point3d.x() = point->X * header->x_scale_factor - pcd.offset.x();
			point3d.y() = point->Y * header->y_scale_factor - pcd.offset.y();
			point3d.z() = point->Z * header->z_scale_factor - pcd.offset.z();
		}
		else{
			point3d.x() = point->X * header->x_scale_factor;
			point3d.y() = point->Y * header->y_scale_factor;
			point3d.z() = point->Z * header->z_scale_factor;
		}

		// 读取颜色
		Eigen::Vector4i_8 color;
		if (hasColor) {
			if (index == 0 && point->rgb[0] <= 255) {
				is8bit = true;
			}
			color.x() = is8bit ? point->rgb[0]: point->rgb[0] / 257;
			color.y() = is8bit ? point->rgb[1]: point->rgb[1] / 257;
			color.z() = is8bit ? point->rgb[2]: point->rgb[2] / 257;
			color.w() = is8bit ? point->rgb[3] : point->rgb[3] / 257;

			pcd.colors.col(index) = color;
		}
		// 不含颜色信息则赋白模
		else {
			color.x() =  255;
			color.y() =  255;
			color.z() =  255;
			color.w() =  0;
			pcd.colors.col(index) = color;
		}

		pcd.position.col(index) = point3d;
		pcd.labels(0, index) = (int)point->classification;
		pcd.intensity(0, index) = (int)point->intensity;
		//std::cout << pcd.intensity(0, index) << std::endl;

		index++;
	}

	// 关闭文件
	laszip_close_reader(reader);
	laszip_destroy(reader);

	Eigen::Vector3f minp = pcd.position.rowwise().minCoeff(); // 取出每一列中最小值 得到最小点
	Eigen::Vector3f maxp = pcd.position.rowwise().maxCoeff(); // 取出每一列中最大值 得到最大点
	
	pcd.boundingBox.extend(minp);
	pcd.boundingBox.extend(maxp);

	pcd.points_num = points_num;

	// Max Min Idensity
	pcd.setmaxIdensity(pcd.intensity.rowwise().maxCoeff()(0, 0));
	pcd.setminIdensity(pcd.intensity.rowwise().minCoeff()(0, 0));

	int64_t end = Now_ms();
	std::cout << "Time of Load File: "<< end - start << "ms" << std::endl;
	return pcd;
}