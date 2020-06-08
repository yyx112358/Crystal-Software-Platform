#pragma once

#include <QTableView>

class ParamView : public QTableView
{
	Q_OBJECT

public:
	enum Role
	{
		INPUT,
		OUTPUT,
		PARAMETER,
	};

	ParamView(QWidget *parent);
	~ParamView();


};
