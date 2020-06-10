#include "stdafx.h"
#include "FixedUI_DEMO.h"
#include <QDebug>
#include <vld.h>
#include "GraphError.h"
#include "CustomTypes.h"

#include "ImageLoader_Dir.h"
#include "ParamWidget.h"
#include "MatGraphicsview.h"

QtPrivate::GraphWarning_StaticHandler GraphWarning::handler;

FixedUI_DEMO::FixedUI_DEMO(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_paramWidgets.append(new ParamWidget(ParamView::INPUT,this));
	_paramWidgets[ParamView::INPUT]->SetName(QStringLiteral("输入"));
	_paramWidgets.append(new ParamWidget(ParamView::OUTPUT, this));
	_paramWidgets[ParamView::OUTPUT]->SetName(QStringLiteral("输出"));
	_paramWidgets.append(new ParamWidget(ParamView::PARAMETER, this));
	_paramWidgets[ParamView::PARAMETER]->SetName(QStringLiteral("运行参数"));
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::INPUT]);
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::OUTPUT]);
	this->addDockWidget(Qt::RightDockWidgetArea, _paramWidgets[ParamView::PARAMETER]);
// 	this->tabifyDockWidget(_paramWidgets[ParamView::INPUT], _paramWidgets[ParamView::OUTPUT]);
// 	this->tabifyDockWidget(_paramWidgets[ParamView::OUTPUT], _paramWidgets[ParamView::PARAMETER]);
	connect(_paramWidgets[ParamView::INPUT]->view, &ParamView::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);
	connect(_paramWidgets[ParamView::OUTPUT]->view, &ParamView::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);
	connect(_paramWidgets[ParamView::PARAMETER]->view, &ParamView::sig_ActionTriggered, this, &FixedUI_DEMO::ParseParamAction);

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
	connect(ui.actionRun, &QAction::triggered, this, &FixedUI_DEMO::Run);
}

FixedUI_DEMO::~FixedUI_DEMO()
{
	_imageLoaders.clear();
	_watchers.clear();
}


void FixedUI_DEMO::SelectAlgorithm(QString name)
{
	if(name == QStringLiteral("阈值化分割"))
	{		
		_paramWidgets[ParamView::INPUT]->view->AddParam("input", MatTypeId(), QStringLiteral("输入图像"));

		_paramWidgets[ParamView::OUTPUT]->view->AddParam("output", MatTypeId(), QStringLiteral("输出图像"), QVariant::fromValue<cv::Mat>(cv::Mat::zeros(3, 4, CV_8U)));
		_paramWidgets[ParamView::OUTPUT]->view->AddParam("threshold", QVariant::Int, QStringLiteral("阈值"));
		
		_paramWidgets[ParamView::PARAMETER]->view->AddParam("threshold1", QVariant::Int, QStringLiteral("阈值"), 7);
		_paramWidgets[ParamView::PARAMETER]->view->AddParam("isDisplay", QVariant::Bool, QStringLiteral("阈值"), false);
		_paramWidgets[ParamView::PARAMETER]->view->AddParam("offset", QVariant::Point, QStringLiteral("阈值"), QPoint(3, 3));
	}
}


void FixedUI_DEMO::AddParamLoader(ParamView&view, QString name, QStandardItem*paramValue)
{
	 auto pimageLoader_Dir = QSharedPointer<ImageLoader_Dir>::create(this);
	 _imageLoaders[paramValue] = pimageLoader_Dir;
	 this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pimageLoader_Dir.get(), Qt::Vertical);
	 //setCentralWidget(nullptr);
}

void FixedUI_DEMO::AddParamWatcher(ParamView&view, QString name, QStandardItem*paramValue)
{
	QSharedPointer<QDockWidget>container = QSharedPointer<QDockWidget>::create(this);
	container->setObjectName(name);
	container->setWindowTitle(name);
	container->resize(400, 300);
	QWidget*widget = new QWidget(container.data());
	auto containerLayout = new QVBoxLayout(widget);
	containerLayout->setSpacing(0);
	containerLayout->setObjectName(QString::fromUtf8("containerLayout"));
	containerLayout->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(containerLayout);

	//TODO:改为Interface_Watcher实现多类型监视
	auto matView = new MatGraphicsview(widget);
	matView->setObjectName("matView");
	if (paramValue->data().canConvert<cv::Mat>())
		matView->SetImg(paramValue->data().value<cv::Mat>());
	containerLayout->addWidget(matView);
	container->setWidget(widget);
	
	connect(&view, &ParamView::sig_ParamChanged, this, [this](QStandardItem*paramValue)
	{
		if (paramValue == nullptr)return;
		if (paramValue->column() == ParamView::VALUE
			&& _watchers.count(paramValue) > 0
			&& _watchers[paramValue].isNull() == false)
		{
			auto container = _watchers[paramValue];
			if (paramValue->data(Qt::ItemDataRole::EditRole).canConvert<cv::Mat>())
			{
				auto matView = container->findChild<MatGraphicsview*>("matView");
				matView->SetImg(paramValue->data(Qt::ItemDataRole::EditRole).value<cv::Mat>());
			}
		}
	}, Qt::UniqueConnection);
	connect(&view, &ParamView::sig_ParamRemoved, this, [this](QStandardItem*paramValue)
	{
		if (paramValue == nullptr)return;
		if (paramValue->column() == ParamView::VALUE)
		{
			_watchers.remove(paramValue);
		}
	}, Qt::UniqueConnection);
	connect(container.data(), &QDockWidget::destroyed, [this](QObject*obj)
	{
		for (auto &c : _watchers)
			if(c==obj)
				c.clear();
	});
// 	if(_watchers.size()==0)
// 		addDockWidget(Qt::BottomDockWidgetArea, container.data(), Qt::Horizontal);
// 	else 
	{
		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, container.data(), Qt::Horizontal);
		//splitDockWidget(container.data(),_paramWidgets[ParamView::PARAMETER],  Qt::Horizontal);
	}
	_watchers.insert(paramValue, container);
}

void FixedUI_DEMO::ParseParamAction(QString actionName, QModelIndex index, QVariantList param, bool checked)
{
	ParamView::ROLE role = ParamView::INPUT;
	if (sender() == _paramWidgets[ParamView::INPUT]->view)
		role = ParamView::INPUT;
	else if (sender() == _paramWidgets[ParamView::OUTPUT]->view)
		role = ParamView::OUTPUT;
	else if (sender() == _paramWidgets[ParamView::PARAMETER]->view)
		role = ParamView::PARAMETER;
	else
		return;

	if (actionName== QStringLiteral("连接输入源"))
	{
		static int current = 0;
		bool ok = false;
		static QStringList algorithms({ QStringLiteral("批次文件") ,QStringLiteral("相机") });
		QString result = QInputDialog::getItem(this, QStringLiteral("选择算法"), "",
			algorithms, current, false, &ok);
		if (ok == true && result.isEmpty() == false)
		{
			for (auto i = 0; i < algorithms.size(); i++)
				if (algorithms[i] == result)
					current = i;			
		}
		AddParamLoader(*_paramWidgets[role]->view,
			sender()->objectName() + ":" + param[ParamView::NAME].toString(), 
			qobject_cast<QStandardItemModel*>(_paramWidgets[role]->view->model())->itemFromIndex(index));
	}
	else if (actionName == QStringLiteral("断开输入源"))
	{

	}
	else if (actionName == QStringLiteral("监视"))
	{
		AddParamWatcher(*_paramWidgets[role]->view,
			sender()->objectName() + ":" + param[ParamView::NAME].toString(),
			qobject_cast<QStandardItemModel*>(_paramWidgets[role]->view->model())->itemFromIndex(index));

	}
}

QStandardItemModel* FixedUI_DEMO::_GetModel(int role)
{
	if (role != ParamView::INPUT && role != ParamView::OUTPUT && role != ParamView::PARAMETER)
		return nullptr;
	auto view = _paramWidgets[role]->view;
	if (view == nullptr)return nullptr;

	QStandardItemModel*model = qobject_cast<QStandardItemModel*>(_paramWidgets[role]->view->model());
	return model;
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
			AddParamWatcher(*_paramWidgets[ParamView::INPUT]->view,
				QStringLiteral("输入") + ":" + "input",
				qobject_cast<QStandardItemModel*>(_paramWidgets[ParamView::INPUT]->view->model())->item(0, ParamView::VALUE));
			AddParamWatcher(*_paramWidgets[ParamView::OUTPUT]->view,
				QStringLiteral("输出") + ":" + "output",
				qobject_cast<QStandardItemModel*>(_paramWidgets[ParamView::OUTPUT]->view->model())->item(0, ParamView::VALUE));
		}
		break;
		case 2:
		{
			AddParamLoader(*_paramWidgets[ParamView::INPUT]->view,
				QStringLiteral("输入") + ":" + "input",
				qobject_cast<QStandardItemModel*>(_paramWidgets[ParamView::INPUT]->view->model())->item(0, ParamView::VALUE));
		}
		break;
		case 3:
		{
			QSharedPointer<ImageLoader_Dir>pimgLoader = (*_imageLoaders.begin()).dynamicCast<ImageLoader_Dir>();

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
		case 5:
		{
			Run();
		}
		break;
		case 6:
		{
			Run();
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
#ifdef _DEBUG
		e.raise();
#endif // _DEBUG
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
#include <opencv.hpp>
#include <QtConcurrent>
void FixedUI_DEMO::Run()
{
	auto inputView = _paramWidgets[ParamView::INPUT]->view,
		outputView = _paramWidgets[ParamView::OUTPUT]->view,
		paramView = _paramWidgets[ParamView::PARAMETER]->view;
	
	while (1)
	{
		bool isEnd = false;
		for (auto loader : _imageLoaders)
		{
			if (loader->IsOpen() == false || loader->IsEnd() == true) 
			{
				isEnd = true;
				break;
			}
		}
		if (isEnd)break;
		for (auto it=_imageLoaders.begin();it!=_imageLoaders.end();++it)
		{
			auto item = it.key();
			auto loader = *it;
			inputView->SetParam(inputView->GetRowName(item), loader->Get(nullptr));
		}

		cv::Mat img = inputView->GetParam("input").value<cv::Mat>();
		cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
		int threshold = paramView->GetParam("threshold").value<int>();
		int th = cv::threshold(img, img, threshold, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
		
		outputView->SetParam("output", QVariant::fromValue(img));
		outputView->SetParam("threshold", th);
		QApplication::processEvents();
	}


}
