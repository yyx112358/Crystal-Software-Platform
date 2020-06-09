#pragma once

#include <QTableView>
class QStandardItemModel;

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

	ParamView(QWidget *parent, ROLE role);
	~ParamView();

	void AddParam(QString name, QVariant::Type type, QString explaination = "", QVariant defaultValue = QVariant());
	void RemoveParam(QString name);
	void SetParam(QString name, QVariant value);
	QVariant GetParam(QString name)const;

	const ROLE _role;

signals:
	void sig_ActionTriggered(QString actionName, QModelIndex index, QVariantList param, bool checked);
private:
	QStandardItemModel _model;//ģ�ͣ��洢�����õġ������Ҫһ��ģ�Ͷ����ͼ����Ҫ������

protected:
	virtual void contextMenuEvent(QContextMenuEvent *) override;

};
