#pragma once

#include <QDockWidget>
#include "ui_ParamViewer.h"
#include "ParamView.h"

class ParamViewer : public QDockWidget
{
	Q_OBJECT

public:
	ParamViewer(QWidget *parent = Q_NULLPTR);
	~ParamViewer();

	void AddParam(ParamView::ROLE role, QString name, QVariant::Type type, QString explaination = "", QVariant defaultValue = QVariant());
	void RemoveParam(ParamView::ROLE role, QString name);
	void SetParam(ParamView::ROLE role, QString name, QVariant value);
	QVariant GetParam(ParamView::ROLE role, QString name)const;
private:
	ParamView& _Role2View(ParamView::ROLE role);
	const ParamView& _Role2View(ParamView::ROLE role)const;

	Ui::ParamViewer ui;
};
