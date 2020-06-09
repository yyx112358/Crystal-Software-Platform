#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FixedUI_DEMO.h"
#include <QSharedPointer>
#include "Interface_ImageLoader.h"


enum ROLE;
class ParamWidget;
class ParamView;

class FixedUI_DEMO : public QMainWindow
{
	Q_OBJECT

public:
	FixedUI_DEMO(QWidget *parent = Q_NULLPTR);
	~FixedUI_DEMO();

	void Debug();
	void Run();
	void SelectAlgorithm(QString name);

	void AddParamLoader(ParamView&view, QString name, QStandardItem*paramValue);
	void AddParamWatcher(ParamView&view, QString name, QStandardItem*paramValue);

	void ParseParamAction(QString actionName, QModelIndex index, QVariantList param, bool checked);
private:
	Ui::FixedUI_DEMOClass ui;
	QList<ParamWidget*>_paramWidgets;//������������
	QHash<QStandardItem*, QSharedPointer<Interface_ImageLoader>>_imageLoaders;//ͼ�����봰��	
	QHash<QStandardItem*, QSharedPointer<QDockWidget>>_watchers;//���Ӵ��ڣ�TODO:���ڸĳ�һ��Interface_Watcher
};
