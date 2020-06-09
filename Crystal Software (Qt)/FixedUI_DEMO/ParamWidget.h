#pragma once

#include <QDockWidget>
#include "ui_ParamWidget.h"
#include "ParamView.h"

class ParamWidget : public QDockWidget
{
	Q_OBJECT

public:
	ParamWidget(ParamView::ROLE role, QWidget *parent = Q_NULLPTR);
	~ParamWidget();

	void AddParam(QString name, QVariant::Type type, QString explaination = "", QVariant defaultValue = QVariant());
	void RemoveParam(QString name);
	void SetParam(QString name, QVariant value);
	QVariant GetParam(QString name)const;

	const ParamView::ROLE role;

signals:
	void sig_ActionTriggered(QString actionName, QModelIndex index, QVariantList param, bool checked);

private:

	Ui::ParamWidget ui;
};
