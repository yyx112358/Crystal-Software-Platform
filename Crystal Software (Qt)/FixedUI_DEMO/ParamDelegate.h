#pragma once

#include <QStyledItemDelegate>

class ParamDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ParamDelegate(QObject *parent);
	~ParamDelegate();
};
