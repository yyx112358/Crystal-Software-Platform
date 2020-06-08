#include "stdafx.h"
#include "FixedUI_DEMO.h"
#include <QDebug>
#include <vld.h>
#include "GraphError.h"
#include "CustomTypes.h"

#include "ImageLoader_Dir.h"
#include "ParamViewer.h"

QtPrivate::GraphWarning_StaticHandler GraphWarning::handler;

FixedUI_DEMO::FixedUI_DEMO(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto pimageLoader_Dir = QSharedPointer<ImageLoader_Dir>::create(this);
	_imageLoaders.append(pimageLoader_Dir);
	this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pimageLoader_Dir.get());

	_paramViewer=new ParamViewer(this);
	this->addDockWidget(Qt::RightDockWidgetArea, _paramViewer);

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
		_paramViewer->AddParam(ParamView::PARAMETER, "threshold1", QVariant::Int, QStringLiteral("阈值"), 7);
		_paramViewer->AddParam(ParamView::PARAMETER, "isDisplay", QVariant::Bool, QStringLiteral("阈值"), false);
		_paramViewer->AddParam(ParamView::PARAMETER, "offset", QVariant::Point, QStringLiteral("阈值"), QPoint(3, 3));
		
		_paramViewer->AddParam(ParamView::INPUT, "input", MatTypeId(), QStringLiteral("输入图像"));

		_paramViewer->AddParam(ParamView::OUTPUT, "output", MatTypeId(), QStringLiteral("输出图像"), QVariant::fromValue<cv::Mat>(cv::Mat::zeros(3, 4, CV_8U)));
		_paramViewer->AddParam(ParamView::OUTPUT, "threshold", QVariant::Int, QStringLiteral("阈值"));
	}
}


void FixedUI_DEMO::ParseParamAction(QString actionName, ROLE role, QVariantList param, bool checked)
{
	if(param[ParamView::TYPE].toInt()==MatTypeId())
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
