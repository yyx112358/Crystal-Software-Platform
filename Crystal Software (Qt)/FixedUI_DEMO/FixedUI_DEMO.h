#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FixedUI_DEMO.h"
#include <QSharedPointer>
#include "Interface_ImageLoader.h"


enum ROLE;
class ParamWidget;

class FixedUI_DEMO : public QMainWindow
{
	Q_OBJECT

public:
	FixedUI_DEMO(QWidget *parent = Q_NULLPTR);
	~FixedUI_DEMO();

	void Debug();
	void SelectAlgorithm(QString name);

	void ParseParamAction(QString actionName, QModelIndex index, QVariantList param, bool checked);
private:
	Ui::FixedUI_DEMOClass ui;
	QList<QSharedPointer<Interface_ImageLoader>>_imageLoaders;
	QList<ParamWidget*>_paramWidgets;
};
