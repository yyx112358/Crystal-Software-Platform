#include "stdafx.h"
#include "AlgNode_DEMO.h"
#include "GuiNode.h"
#include <opencv.hpp>
#include "CustomTypes.h"

using namespace cv;
using namespace std;

AlgNode_DEMO_InputImage::AlgNode_DEMO_InputImage(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	:AlgNode_Input(pool,parent)
{
	_mode = AlgNode::RunMode::Thread;
#ifdef _DEBUG
	QTimer::singleShot(100, [this] {
		static int i = 0;
		if(i==0)
			SetImagePath(QStringLiteral("F:\\硕士相关\\实验\\结晶实验\\2019-09-05\\NO02_2019-09-22 13hh 48min 41sec475ms.Jpeg"));
		else if(i==1)
			SetImagePath(QStringLiteral("F:\\opencv\\Doc_4.0.0\\lena.png"));
		else if(i==2)
			SetImagePath(QStringLiteral("H:\\Libraries\\OpenCV\\4.1.1\\sources\\doc\\opencv.jpg"));
		i++;
	});	
#endif // _DEBUG
}

void AlgNode_DEMO_InputImage::Init()
{
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "img", QVariant::fromValue(cv::Mat()));
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "type", 0);
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "size", QSize(0, 0));
}

void AlgNode_DEMO_InputImage::SetImagePath(QString path)
{
	if (_gui.isNull() == false)
		_gui->SetData(path);
}

QVariantHash AlgNode_DEMO_InputImage::_Run(QVariantHash data)
{
	GRAPH_ASSERT(_gui.isNull() == false);
#ifdef _DEBUG
	QThread::msleep(500);
#endif // _DEBUG
	data.clear();
	QString filename = _gui->GetData().toString();
	Mat img = imread(filename.toLocal8Bit().toStdString());
	if (img.empty() == true)
		return data;	
	data["img"] = QVariant::fromValue<cv::Mat>(img);
	data["type"] = img.type();
	data["size"] = QSize(img.cols, img.rows);
	return data;
}



AlgNode_DEMO_ShowImage::AlgNode_DEMO_ShowImage(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	:AlgNode_Output(pool, parent)
{
	_mode = AlgNode::RunMode::Thread;
}

void AlgNode_DEMO_ShowImage::Init()
{
	AddVertexAuto(AlgVertex::VertexType::INPUT, "img", QVariant::fromValue(cv::Mat()));
}
QVariantHash AlgNode_DEMO_ShowImage::_Run(QVariantHash data)
{
	GRAPH_ASSERT(_gui.isNull() == false);
#ifdef _DEBUG
	QThread::msleep(500);
#endif // _DEBUG
	cv::Mat img = data.value("img", QVariant::fromValue(cv::Mat())).value<cv::Mat>();//转换，如果类型不对或不存在img项将返回空img
	_gui->SetData(Mat2QPixmap(img));
	return data;
}

QVariant GuiNode_DEMO_ShowImage::GetData()
{
	GRAPH_ASSERT(_panel.isNull() == false);
	auto label = _panel.objectCast<QLabel>();
	if (label->pixmap() != nullptr)
		return *label->pixmap();
	else
		return QPixmap();
}

void GuiNode_DEMO_ShowImage::SetData(QVariant data)
{
	GRAPH_ASSERT(_panel.isNull() == false && (data.canConvert<QPixmap>() == true || data.canConvert<cv::Mat>() == true));
	auto label = _panel.objectCast<QLabel>();
	if (data.canConvert<QPixmap>() == true) 
	{
		QPixmap pix = data.value<QPixmap>();
		if (pix.width() > label->width())
			pix=pix.scaledToWidth(/*label->width()*/200);
		label->setPixmap(pix);
	}
	else if (data.canConvert<cv::Mat>() == true)
		label->setPixmap(Mat2QPixmap(data.value<cv::Mat>()));
	else
		label->setText("");
}


AlgNode_DEMO_ImageROI::AlgNode_DEMO_ImageROI(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	:AlgNode(pool,parent)
{
	_mode = AlgNode::RunMode::Direct;

}

void AlgNode_DEMO_ImageROI::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "src", AlgVertex::Behavior_NoData::USE_LAST,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::KEEP, QVariant::fromValue(cv::Mat()));
	AddVertex(AlgVertex::VertexType::INPUT, "x", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "y", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "size", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, QSize());

	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "dst", QVariant::fromValue(cv::Mat()));
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "rect", QRect());
}

QVariantHash AlgNode_DEMO_ImageROI::_Run(QVariantHash data)
{
	cv::Mat img = data["src"].value<cv::Mat>();
	bool ok = false;
	cv::Rect roi(data["x"].toInt(&ok), data["y"].toInt(&ok), data["size"].toSize().width(), data["size"].toSize().height());
	//GRAPH_ASSERT(ok==true && )
	QVariantHash result;
	try
	{
		result["dst"] = QVariant::fromValue(img(roi));
		result["rect"] = QRect(roi.x, roi.y, roi.width, roi.height);
		return result;
	}
	catch (...)
	{
		return result;
	}
}

void AlgNode_DEMO_AddWeighted::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "imgA", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, QVariant::fromValue(cv::Mat()));
	AddVertex(AlgVertex::VertexType::INPUT, "imgB", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "alpha", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0.5);
	AddVertex(AlgVertex::VertexType::INPUT, "beta", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0.5);
	AddVertex(AlgVertex::VertexType::INPUT, "gamma", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);

	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "dst", QVariant::fromValue(cv::Mat()));
}

AlgNode_DEMO_AddWeighted::AlgNode_DEMO_AddWeighted(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
	:AlgNode(pool, parent) 
{
	_mode = AlgNode::RunMode::Thread;

}

QVariantHash AlgNode_DEMO_AddWeighted::_Run(QVariantHash data)
{
#ifdef _DEBUG
	QThread::msleep(500);
#endif // _DEBUG
	cv::Mat dst;
	cv::addWeighted(data["imgA"].value<cv::Mat>(), data["alpha"].toDouble(),
		data["imgB"].value<cv::Mat>(), data["beta"].toDouble(), data["gamma"].toDouble(), dst);
	QVariantHash result;
	result["dst"] = QVariant::fromValue(dst);
	return result;
}

void AlgNode_DEMO_SetROI::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "imgA", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, QVariant::fromValue(cv::Mat()));
	AddVertex(AlgVertex::VertexType::INPUT, "imgB", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "x", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "y", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);

	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "dst", QVariant::fromValue(cv::Mat()));
}

QVariantHash AlgNode_DEMO_SetROI::_Run(QVariantHash data)
{
#ifdef _DEBUG
	QThread::msleep(500);
#endif // _DEBUG
	cv::Mat imgA = data["imgA"].value<cv::Mat>(), imgB = data["imgB"].value<cv::Mat>(), dst=imgA.clone();
	imgB.copyTo(dst(cv::Rect(data["x"].toInt(), data["y"].toInt(), imgB.cols, imgB.rows)));
	QVariantHash result;
	result["dst"] = QVariant::fromValue(dst);
	return result;
}

void AlgNode_DEMO_BiggerThan::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "data", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, QVariant());
	AddVertex(AlgVertex::VertexType::INPUT, "cmpA", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "cmpB", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, 0);

	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "true", QVariant());
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "false", QVariant());
}

QVariantHash AlgNode_DEMO_BiggerThan::_Run(QVariantHash data)
{
	QVariantHash result;
	if (data["cmpA"] > data["cmpB"])
		result["true"] = data["data"];
	else
		result["false"] = data["data"];
	return result;
}

void AlgNode_DEMO_Count::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "trig", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, QVariant());

	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "count", _cnt);
}

void AlgNode_DEMO_Count::Reset()
{
	_cnt = 0;
	AlgNode::Reset();
}

QVariantHash AlgNode_DEMO_Count::_Run(QVariantHash data)
{
	QVariantHash result;
	result["count"] = _cnt;
	_cnt++;
	return result;
}

void AlgNode_DEMO_Switch::Init()
{
	AddVertex(AlgVertex::VertexType::INPUT, "data1", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "case1", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);

	AddVertex(AlgVertex::VertexType::INPUT, "data2", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "case2", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);

	AddVertex(AlgVertex::VertexType::INPUT, "default", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);
	AddVertex(AlgVertex::VertexType::INPUT, "num", AlgVertex::Behavior_NoData::USE_NULL,
		AlgVertex::Behavior_BeforeActivate::BUFFER, AlgVertex::Behavior_AfterActivate::RESET, 0);
	
	AddVertexAuto(AlgVertex::VertexType::OUTPUT, "out", QVariant());
}

QVariantHash AlgNode_DEMO_Switch::_Run(QVariantHash data)
{
#ifdef _DEBUG
	QThread::msleep(500);
#endif // _DEBUG
	QVariantHash result;
	if (data["case1"].toInt() == data["num"].toInt())
		result["out"] = data["data1"];
	else if (data["case2"].toInt() == data["num"].toInt())
		result["out"] = data["data2"];
	else
		result["out"] = data["default"];
	return result;
}
