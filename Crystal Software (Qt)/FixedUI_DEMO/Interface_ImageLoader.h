#pragma once

class QVariant;
template QHash<QString, QVariant>;//class QVariantHash;
//template QList<QVariant>;
class QString;
template QList<QString>;

class Interface_ImageLoader
{
public:
	virtual bool Load(QStringList path) = 0;
	virtual void Clear() = 0;

	//virtual QVariantList Get(size_t begin, size_t end = UINT_MAX) = 0;
	virtual QVariant Get(QVariantHash*extraInfo = nullptr) = 0; // ��ȡһ��ͼ��extraInfo��������Ϣ
	virtual bool Seek(size_t idx) = 0;//��ָ���Ƶ���idx���ڵ㣬�������л�

	virtual int size()const = 0;//ͼ�����ݼ�������-1�����������
	virtual int Pos()const = 0;//��ǰλ��
	virtual bool IsOpen()const = 0;
	virtual bool IsEnd()const = 0;

	virtual QVariantHash SetSetting(QVariantHash setting) = 0;//�����趨������ֵΪδ�ɹ��趨�Ĳ���
	virtual QVariantHash GetSetting()const = 0;
};