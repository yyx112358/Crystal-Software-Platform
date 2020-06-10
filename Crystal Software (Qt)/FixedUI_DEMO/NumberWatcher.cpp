#include "stdafx.h"
#include "NumberWatcher.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

NumberWatcher::NumberWatcher(QWidget *parent)
	: QDockWidget(parent),_widget(new QWidget)
{
	setObjectName(QString::fromUtf8("NumberWatcher"));
	resize(400, 300);

	_widget->setObjectName(QString::fromUtf8("_widget"));
	_layout = new QVBoxLayout(_widget);
	_layout->setSpacing(0);
	_layout->setObjectName(QString::fromUtf8("_layout"));
	_layout->setContentsMargins(0, 0, 0, 0);
	_chartView = new QChartView(_widget);
	_chartView->setObjectName(QString::fromUtf8("_chartView"));

	_layout->addWidget(_chartView);

	setWidget(_widget);

	_chart = new QtCharts::QChart;
	_line = new QtCharts::QLineSeries;
	_axisX = new QtCharts::QValueAxis;
	_axisY = new QtCharts::QValueAxis;

	_chartView->setChart(_chart);
	_chartView->setRenderHint(QPainter::RenderHint::Antialiasing);//抗锯齿
	_chart->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);//动画选项
	_chart->legend()->hide();//图例
	_chart->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);//主题
	auto pal = window()->palette();
	pal.setColor(QPalette::Window, QRgb(0x40434a));
	pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));

	_chart->addSeries(_line);
	_line->pen().setColor(Qt::GlobalColor::blue);
	_line->pen().setWidth(3);
	//	_line.append(0, 0);

	_axisX->setTickCount(6);
	_chart->setAxisX(_axisX, _line);
	_axisX->setRange(0, 60);
	_chart->setAxisY(_axisY, _line);
	_axisY->setRange(0, 256);
}

NumberWatcher::~NumberWatcher()
{
}

bool NumberWatcher::Reset()
{
	_line->clear();
	return true;
}

bool NumberWatcher::AppendParam(QVariant param)
{
	if (AssertInput(param) == false)
		return false;
	bool ok = false;
	int valueX = _line->points().size() + 1;
	int valueY = param.toDouble(&ok);
	if (ok == false)
		return false;

	_line->append(valueX, valueY);
	//自动卷动
	float dx = _chart->plotArea().width() / (_axisX->max() - _axisX->min());
	if (valueX > _axisX->max()*0.8)
		_chart->scroll(dx, 0);
	return true;
}

bool NumberWatcher::AppendParam(QList<QVariant>params)
{
	throw std::logic_error("The method or operation is not implemented.");
}

QVariant NumberWatcher::GetParam(int idx) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

QList<QVariant> NumberWatcher::GetParam() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

int NumberWatcher::type() const
{
	return QVariant::Type::Double;
}

bool NumberWatcher::IsSupportType(int type) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void NumberWatcher::SetIsSave(bool isSave, int maximum /*= 2147483647*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool NumberWatcher::AssertInput(QVariant var)const
{
	if (var.isValid() == true && var.canConvert(type()) == true)
		return true;
	else
		return false;
}
