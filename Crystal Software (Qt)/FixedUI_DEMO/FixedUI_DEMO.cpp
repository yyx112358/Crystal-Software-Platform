#include "stdafx.h"
#include "FixedUI_DEMO.h"
#include "ImageLoader_Dir.h"
#include <QDebug>
#include <vld.h>
#include "GraphError.h"
#include "CustomTypes.h"


QtPrivate::GraphWarning_StaticHandler GraphWarning::handler;

FixedUI_DEMO::FixedUI_DEMO(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto pimageLoader_Dir = QSharedPointer<ImageLoader_Dir>::create(this);
	_imageLoaders.append(pimageLoader_Dir);
	this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pimageLoader_Dir.get());
	connect(ui.actionDebug, &QAction::triggered, this, &FixedUI_DEMO::Debug);
}

FixedUI_DEMO::~FixedUI_DEMO()
{
	_imageLoaders.clear();
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
			QSharedPointer<ImageLoader_Dir>pimgLoader = _imageLoaders.back().dynamicCast<ImageLoader_Dir>();
			GRAPH_ASSERT(pimgLoader->Load({ "d:\\Users\\yyx11\\Desktop\\Saved Pictures\\BRISQUE_2020-02-10\\2019-01-24 12hh 28min 39sec925ms.Jpeg.13.png",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO02_2019-09-22 13hh 48min 41sec475ms.Jpeg",
				"d:/Users/yyx11/Desktop/Saved Pictures/crystal_dataset/crystal_20190905/NO06_2019-09-05 09hh 19min 47sec448ms.Jpeg",
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
		case 1:
		{
			QSharedPointer<ImageLoader_Dir>pimgLoader = _imageLoaders.back().dynamicCast<ImageLoader_Dir>();
			QVariantHash extraInfo;
			QVariant imgVar = pimgLoader->Get(&extraInfo);
			qDebug() << extraInfo;
			GRAPH_ASSERT(imgVar.isNull()==false);
			cv::Mat img = imgVar.value<cv::Mat>();
			cv::Mat testimg = cv::imread(extraInfo["path"].toString().toStdString());
			GRAPH_ASSERT(cv::countNonZero(img != testimg) == 0);
			cv::imshow(__FUNCTION__, img);
			cv::waitKey();
			cv::destroyWindow(__FUNCTION__);
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
