#pragma once

#include <QTableView>
class QStandardItemModel;
class QStandardItem;

class ParamView : public QTableView
{
	Q_OBJECT

public:
	enum ROLE
	{
		INPUT,
		OUTPUT,
		PARAMETER,
	};//��ɫ
	enum COLUMN
	{
		STATUS=0,
		NAME=1,
		VALUE=2,
		TYPE=3,
		EXPLAINATION=4,
	};//�����м����
	enum ACTION
	{
		CONNECT_SOURCE,//�����������Դ
		MONITOR,//����
	};

	ParamView(QWidget *parent);
	~ParamView();

	void AddParam(QString name, QVariant::Type type, QString explaination = "", QVariant defaultValue = QVariant());
	void RemoveParam(QString name);

	void SetParam(QString name, QVariant value);
	QVariant GetParam(QString name)const;

	QString GetRowName(QStandardItem*item)const;
	int GetRow(QString name)const;
	int GetStatus(QString name);
	QVariant::Type GetType(QString name)const;
	QString GetExplaination(QString name)const;
	QStandardItem*GetValueItem(QString name);

	const ROLE _role;//��ɫ�����������д�ɺ�������Ϊ��������ط�����ParamWidget()���캯���б�ǿ�и�ֵ

signals:
	void sig_ActionTriggered(QString actionName, QModelIndex index, QVariantList param, bool checked);
	void sig_ParamAdded(QStandardItem*paramValue);
	void sig_ParamChanged(QStandardItem*paramValue);
	void sig_ParamRemoved(QStandardItem*paramValue);
private:
	QStandardItemModel _model;//ģ�ͣ��洢�����õġ������Ҫһ��ģ�Ͷ����ͼ����Ҫ������

protected:
	virtual void contextMenuEvent(QContextMenuEvent *) override;

};
