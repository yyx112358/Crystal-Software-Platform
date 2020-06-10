#pragma once

#include <QDockWidget>
//#include "ui_NumberWatcher.h"
#include "Interface_ParamWatcher.h"

class QVBoxLayout;
class QString;
namespace QtCharts
{
	class QChart;
	class QLineSeries;
	class QValueAxis;
	class QChartView;
}

class NumberWatcher : public QDockWidget,public Interface_ParamWatcher
{
	Q_OBJECT

public:
	NumberWatcher(QWidget *parent = Q_NULLPTR);
	~NumberWatcher();


	virtual bool Reset() override;
	virtual bool AppendParam(QVariant param) override;
	virtual bool AppendParam(QList<QVariant>params) override;
	virtual QVariant GetParam(int idx) const override;
	virtual QList<QVariant> GetParam() const override;

	virtual int type() const override;
	virtual bool IsSupportType(int type) const override;

	virtual void SetIsSave(bool isSave, int maximum = 2147483647) override;

private:
	bool AssertInput(QVariant&var);

	QWidget*_widget = nullptr;
	QVBoxLayout*_layout = nullptr;

	QtCharts::QChartView *_chartView = nullptr;
	QtCharts::QChart *_chart = nullptr;
	QtCharts::QLineSeries *_line = nullptr;
	QtCharts::QValueAxis *_axisX = nullptr, *_axisY = nullptr;

	bool isSave = true;
	int maximum = 2147483647;
};
