/*
 * @Descripttion: 
 * @version: 
 * @Author: JinYiGao
 * @Date: 2021-01-30 23:03:53
 * @LastEditors: JinYiGao
 * @LastEditTime: 2021-01-30 23:03:53
 */
#pragma once
#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QTextStream>
#include <QTranslator>

//void load_style() {
//	//QFile f(":qdarkstyle/style.qss");
//	QFile f(":qstyle/Style_Gray.qss");
//	f.open(QFile::ReadOnly | QFile::Text);
//	QTextStream ts(&f);
//	qApp->setStyleSheet(ts.readAll());
//
//	/*int fontid = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/wqy-microhei.ttc"));
//	QString wqy = QFontDatabase::applicationFontFamilies(fontid).at(0);
//	QFont font(wqy);
//#if _WIN32
//	font.setPointSizeF(9.0);
//#else
//	font.setPointSizeF(12.0);
//#endif
//	QApplication::setFont(font);*/
//}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//load_style();

    MainWindow w;
	w.resize(QSize(1100, 800));
    w.show();

    return a.exec();
}
