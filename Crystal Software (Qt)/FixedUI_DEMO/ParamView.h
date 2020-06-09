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
	};//角色
	enum COLUMN
	{
		STATUS=0,
		NAME=1,
		VALUE=2,
		TYPE=3,
		EXPLAINATION=4,
	};//各个列及编号
	enum ACTION
	{
		CONNECT_SOURCE,//连接输入输出源
		MONITOR,//监视
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

	const ROLE _role;//角色。理论上最好写成函数，但为方便这个地方会在ParamWidget()构造函数中被强行赋值

signals:
	void sig_ActionTriggered(QString actionName, QModelIndex index, QVariantList param, bool checked);
	void sig_ParamAdded(QStandardItem*paramValue);
	void sig_ParamChanged(QStandardItem*paramValue);
	void sig_ParamRemoved(QStandardItem*paramValue);
private:
	QStandardItemModel _model;//模型，存储数据用的。如果需要一套模型多个视图则需要将其抽出

protected:
	virtual void contextMenuEvent(QContextMenuEvent *) override;

};
