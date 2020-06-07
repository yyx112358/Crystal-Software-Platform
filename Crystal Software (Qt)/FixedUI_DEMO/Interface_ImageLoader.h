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
	virtual QVariant Get(QVariantHash*extraInfo = nullptr) = 0; // 获取一个图像。extraInfo：额外信息
	virtual bool Seek(size_t idx) = 0;//将指针移到第idx个节点，不反序列化

	virtual int size()const = 0;//图像数据集总数，-1代表个数不定
	virtual int Pos()const = 0;//当前位置
	virtual bool IsOpen()const = 0;
	virtual bool IsEnd()const = 0;

	virtual QVariantHash SetSetting(QVariantHash setting) = 0;//进行设定。返回值为未成功设定的参数
	virtual QVariantHash GetSetting()const = 0;
};