#pragma once
//�����ṩQt Metatype���Զ������͵�֧��
#include <QVariant>
#include <opencv2/core/mat.hpp>
//����metatype Q_DECLARE_METATYPE()
Q_DECLARE_METATYPE(cv::Mat);

QString QVariant2Description(QVariant var, int len = 31);//ת��Ϊ������������len�ǳ������ƣ�<=0��Ϊ������
