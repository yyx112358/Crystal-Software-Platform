#pragma once
//�����ṩQt Metatype���Զ������͵�֧��
#include <QVariant>
#include <opencv2/core/mat.hpp>
//����metatype Q_DECLARE_METATYPE()
Q_DECLARE_METATYPE(cv::Mat);
QVariant::Type MatTypeId();

QString QVariant2Description(QVariant var, int len = 31);//ת��Ϊ������������len�ǳ������ƣ�<=0��Ϊ������
QString MatType2Description(int type);//��cv::Mat.type()ת��Ϊ��������
QPixmap Mat2QPixmap(QVariant m);
QPixmap Mat2QPixmap(cv::Mat m);
cv::Mat QPixmap2Mat(QPixmap pix);