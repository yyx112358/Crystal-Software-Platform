#include "stdafx.h"
#include "FixedUI_DEMO.h"
#include <QDebug>
#include <vld.h>
#include "GraphError.h"
#include "CustomTypes.h"

#include "ImageLoader_Dir.h"
#include "ParamWidget.h"

QtPrivate::GraphWarning_StaticHandler GraphWarning::handler;

FixedUI_DEMO::FixedUI_DEMO(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto pimageLoader_Dir = QSharedPointer<ImageLoader_Dir>::create(this);
	_imageLoaders.append(pimageLoader_Dir);
	this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pimageLoader_Dir.get());

	_paramWidgets.append(new ParamWidget(ParamView::INPUT,this));
	_paramWidgets[ParamView::INPUT]->setWindowTitle(QStringLiteral("输入"));
	_paramWidgets.append(new ParamWidget(ParamView::OUTPUT, this));
	_paramWidgets[ParamView::OUTPUT]->setWindowTitle(QStringLiteral("输出"));
	_paramWidgets.append(new ParamWidget(ParamView::PARAMETER, this));
	_paramWidgets[ParamView::PARAMETER]->setWindowTitle(QStringLiteral("运行参数"));
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::INPUT]);
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::OUTPUT]);
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::PARAMETER]);
// 	this->tabifyDockWidget(_paramWidgets[ParamView::INPUT], _paramWidgets[ParamView::OUTPUT]);
// 	this->tabifyDockWidget(_paramWidgets[ParamView::OUTPUT], _paramWidgets[ParamView::PARAMETER]);
	connect(_paramWidgets[ParamView::INPUT], &ParamWidget::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);
	connect(_paramWidgets[ParamView::OUTPUT], &ParamWidget::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);
	connect(_paramWidgets[ParamView::PARAMETER], &ParamWidget::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);

	connect(ui.actionDebug, &QAction::triggered, this, &FixedUI_DEMO::Debug);
	connect(ui.action_SelectAlgorithm, &QAction::triggered, [this](bool b)
	{
		static int current = 0;
		bool ok = false;
		static QStringList algorithms({ QStringLiteral("阈值化分割") ,QStringLiteral("机器学习分割") });
		QString result = QInputDialog::getItem(this, QStringLiteral("选择算法"), "",
			algorithms, current, false, &ok);
		if (ok == true && result.isEmpty() == false) 
		{
			for (auto i = 0; i < algorithms.size(); i++)
				if (algorithms[i] == result)
					current = i;
			SelectAlgorithm(result);
		}
	});
}

FixedUI_DEMO::~FixedUI_DEMO()
{
	_imageLoaders.clear();
}


void FixedUI_DEMO::SelectAlgorithm(QString name)
{
	if(name == QStringLiteral("阈值化分割"))
	{		
		_paramWidgets[ParamView::INPUT]->AddParam("input", MatTypeId(), QStringLiteral("输入图像"));

		_paramWidgets[ParamView::OUTPUT]->AddParam("output", MatTypeId(), QStringLiteral("输出图像"), QVariant::fromValue<cv::Mat>(cv::Mat::zeros(3, 4, CV_8U)));
		_paramWidgets[ParamView::OUTPUT]->AddParam("threshold", QVariant::Int, QStringLiteral("阈值"));
		
		_paramWidgets[ParamView::PARAMETER]->AddParam("threshold1", QVariant::Int, QStringLiteral("阈值"), 7);
		_paramWidgets[ParamView::PARAMETER]->AddParam("isDisplay", QVariant::Bool, QStringLiteral("阈值"), false);
		_paramWidgets[ParamView::PARAMETER]->AddParam("offset", QVariant::Point, QStringLiteral("阈值"), QPoint(3, 3));
	}
}


void FixedUI_DEMO::ParseParamAction(QString actionName, QModelIndex index, QVariantList param, bool checked)
{
	ParamView::ROLE role = ParamView::INPUT;
	if (sender() == _paramWidgets[ParamView::INPUT])
		role = ParamView::INPUT;
	else if (sender() == _paramWidgets[ParamView::OUTPUT])
		role = ParamView::OUTPUT;
	else if (sender() == _paramWidgets[ParamView::PARAMETER])
		role = ParamView::PARAMETER;
	else
		return;

	if (actionName== QStringLiteral("连接输入源"))
	{
		
	}
	else if (actionName == QStringLiteral("断开输入源"))
	{

	}
	else if (actionName == QStringLiteral("监视"))
	{

	}
}

#include <opencv2/highgui.hpp>
void FixedUI_DEMO::Debug()
{
	static int cnt = 0;

	try
	{
		switch (cnt)
		{
		case 0:
		{
			SelectAlgorithm(QStringLiteral("阈值化分割"));
			}
		break;
		case 1:
		{
			QSharedPointer<ImageLoader_Dir>pimgLoader = _imageLoaders.back().dynamicCast<ImageLoader_Dir>();
			GRAPH_ASSERT(pimgLoader->Load({
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO02_2019-09-22 13hh 48min 41sec475ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 47sec448ms.Jpeg",
				"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\BRISQUE_2020-02-10\\2019-01-24 12hh 28min 39sec925ms.Jpeg.13.png",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 48sec118ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 48sec784ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 49sec447ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 50sec136ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 50sec820ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 51sec505ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 52sec193ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 52sec877ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 53sec565ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 54sec251ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 54sec936ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 55sec622ms.Jpeg", }));

		}
		break;
		default:
			break;
		}
	}
	catch (GraphError&e)
	{
		qDebug() << e.msg;
		GRAPH_INFORM(e.msg);
		QMessageBox::warning(this, __FUNCTION__, e.msg);
	}
	cnt++;

	QMessageBox *msgbox=new QMessageBox(QString(__FUNCTION__), QString("[%1] Click Ok to continue").arg(cnt),
		QMessageBox::Icon::Information,		QMessageBox::StandardButton::Ok ,
		QMessageBox::StandardButton::Cancel, QMessageBox::StandardButton::Reset, this);
	connect(msgbox, &QMessageBox::buttonClicked, [this](QAbstractButton*btn)
	{
		if (btn->text() == "OK")
			Debug();
		else if (btn->text() == "Reset")
			cnt = 0;
	});
	msgbox->setModal(false);
	msgbox->show();
}
