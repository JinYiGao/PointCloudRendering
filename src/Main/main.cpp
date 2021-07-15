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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
	w.resize(QSize(1100, 800));
    w.show();

    return a.exec();
}
