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
	};//角色
	enum COLUMN
	{
		STATUS=0,
		NAME=1,
		VALUE=2,
		TYPE=3,
		EXPLAINATION=4,
	};//各个列及编号

	ParamView(QWidget *parent, ROLE role);
	~ParamView();

	void AddParam(QString name, QVariant::Type type, QString explaination = "", QVariant defaultValue = QVariant());
	void RemoveParam(QString name);
	void SetParam(QString name, QVariant value);
	QVariant GetParam(QString name)const;

	const ROLE _role;
private:
	QStandardItemModel _model;//模型，存储数据用的。如果需要一套模型多个视图则需要将其抽出
};
