#pragma once
//用于提供Qt Metatype对自定义类型的支持
#include <QVariant>

//声明metatype Q_DECLARE_METATYPE()

QString QVariant2Description(QVariant var, int len = 31);//转化为文字性描述，len是长度限制，<=0则为无限制
111