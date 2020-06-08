#pragma once
//用于提供Qt Metatype对自定义类型的支持
#include <QVariant>
#include <opencv2/core/mat.hpp>
//声明metatype Q_DECLARE_METATYPE()
Q_DECLARE_METATYPE(cv::Mat);
QVariant::Type MatTypeId();

QString QVariant2Description(QVariant var, int len = 31);//转化为文字性描述，len是长度限制，<=0则为无限制
QString MatType2Description(int type);//将cv::Mat.type()转化为文字描述
QPixmap Mat2QPixmap(QVariant m);
QPixmap Mat2QPixmap(cv::Mat m);
cv::Mat QPixmap2Mat(QPixmap pix);