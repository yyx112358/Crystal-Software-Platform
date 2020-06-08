/*!
 * \file MatGraphicsView.cpp
 * \date 2018/08/30 19:46
 *
 * \author yyx11
 * Contact: user@company.com
 *
 * \brief 支持cv::Mat的图像显示控件
 *
 * TODO: long description
 *
 * \note
*/

#include "stdafx.h"
#include "MatGraphicsView.h"
#include <vector>
#include <QWheelEvent>
#include <QMenu>
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

#include <opencv2\imgproc.hpp>
#include <opencv2\imgcodecs.hpp>

static cv::Mat PseudoTransform(const cv::Mat&src);

const char MatGraphicsview::supportFileFormats[] =
{
	"Portable Network Graphics(*.png)"
	";;Jpeg Files(*.jpg *.jpeg *.jpe)"
	";;Windows bitmaps(*.bmp *.dib)"
	";;JPEG 2000 files(*.jp2)"
	";;WebP(*.webp)"
	";;Portable image format(*.pbm *.pgm *.ppm *.pxm, *.pnm)"
	";;Sun rasters(*.sr *.ras)"
	";;OpenEXR Image files(*.exr)"
	";;Radiance HDR(*.hdr *.pic)"
};

MatGraphicsview::MatGraphicsview(QWidget *parent)
	: QGraphicsView(parent),
	_menu("", this),
	actionZoom_to_Fit(QStringLiteral("Zoom to Fit"), this),
	actionZoom_to_Original_Size(QStringLiteral("Zoom to Original Size"), this),
	actionRotate_Clockwise_90(QStringLiteral("Rotate Clockwise 90"), this),
	actionRotate_Anticlockwise_90(QStringLiteral("Rotate Anticlockwise 90"), this),
	actionAuto_Maximize_Contrast(QStringLiteral("Auto Maximize Contrast"), this),
	actionPseudo_Color(QStringLiteral("Pseudo Color"), this),
	actionIgnore_Alpha(QStringLiteral("Ignore Alpha"), this),
	actionDraw_Contour(QStringLiteral("Draw Contour"), this),
	actionDump_to_File(QStringLiteral("Dump to File"), this),
	actionLoad_From_File(QStringLiteral("Load From File"),this),
	_menu_Channel(QStringLiteral("Channel"), this),
	actionGroup_Channel(this),
	action_Channel_All(QStringLiteral("All"), this),
	_menu_Convert(QStringLiteral("Convert"), this),
	actionGroup_Convert(this)
{
	_scene.addItem(&_backPixmap);
	setScene(&_scene);
	
	_menu.addAction(&actionZoom_to_Fit);
	connect(&actionZoom_to_Fit, &QAction::triggered, this, &MatGraphicsview::_ZoomToFit);
	_menu.addAction(&actionZoom_to_Original_Size);
	connect(&actionZoom_to_Original_Size, &QAction::triggered, [=] 
	{
		_Zoom(1); 
	});
	_menu.addAction(&actionRotate_Clockwise_90);
	connect(&actionRotate_Clockwise_90, &QAction::triggered, [=]
	{
		this->rotate(90);
	});
	_menu.addAction(&actionRotate_Anticlockwise_90);
	connect(&actionRotate_Anticlockwise_90, &QAction::triggered, [=]
	{
		this->rotate(-90);
	});
	_menu.addSeparator();

	_menu.addAction(&actionAuto_Maximize_Contrast);
	actionAuto_Maximize_Contrast.setCheckable(true);
	connect(&actionAuto_Maximize_Contrast, &QAction::triggered, [=](bool ischecked)
	{
		_setting.isMaxContrast = ischecked;
		_SetImgWithSettings();
	});
	_menu.addAction(&actionPseudo_Color);
	actionPseudo_Color.setCheckable(true);
	connect(&actionPseudo_Color, &QAction::triggered, [=](bool ischecked)
	{
		_setting.isPseudoColor = ischecked;
		_SetImgWithSettings();
	});
	_menu.addAction(&actionIgnore_Alpha);
	actionIgnore_Alpha.setCheckable(true);
	connect(&actionIgnore_Alpha, &QAction::triggered, [=](bool ischecked)
	{
		_setting.isIgnoreAlpha = ischecked;
		_SetImgWithSettings();
	});
	_menu.addAction(&actionDraw_Contour);
	actionDraw_Contour.setCheckable(true);
	actionDraw_Contour.setToolTip("If this image is a 2D point set, use it to draw the contour");
	connect(&actionDraw_Contour, &QAction::triggered, [=](bool ischecked)
	{
		_setting.isContour = ischecked;
		_SetImgWithSettings();
	});
	_menu.addSeparator();

	_menu.addMenu(&_menu_Channel);
	action_Channel_All.setCheckable(true);
	action_Channel_All.setChecked(true);
	_menu_Channel.addAction(&action_Channel_All);
	actionGroup_Channel.addAction(&action_Channel_All);
	connect(&action_Channel_All, &QAction::triggered, [=] 
	{
		_setting.channel = -1;
		_SetImgWithSettings();
	});
	int i = 0;
	for (auto &action : action_Channel)
	{
		action.setText(QString::number(i));
		action.setCheckable(true);
		_menu_Channel.addAction(&action);
		actionGroup_Channel.addAction(&action);
		connect(&action, &QAction::triggered, [=]
		{
			_setting.channel = i;
			_SetImgWithSettings();
		});
		i++;
	}
	_menu.addSeparator();

	_menu.addMenu(&_menu_Convert);
	_menu_Convert.setToolTip("Convert image from [BGR] to selected");
	i = 0;
	for (auto &action : action_Convert)
	{
		action.setText(_setting.colorTypeName[i]);
		action.setCheckable(true);
		_menu_Convert.addAction(&action);
		actionGroup_Convert.addAction(&action);
		connect(&action, &QAction::triggered, [=]
		{
			_setting.colorType = static_cast<S_Settings::E_ColorType>(i);
			_SetImgWithSettings();
		});
		i++;
	}
	action_Convert[_setting.Default].setChecked(true);
	_menu.addSeparator();

	_menu.addAction(&actionDump_to_File);
	connect(&actionDump_to_File, &QAction::triggered, [=]
	{
		QString name = QFileDialog::getSaveFileName(this, tr("Save Current Image"), ".",
			tr(supportFileFormats));

		if (name.isEmpty() == false)
		{
			try
			{
				if (cv::imwrite(name.toStdString(), _procImg) == false)
					throw std::exception("Save file error");
			}
			catch (...)
			{
				QMessageBox::warning(this, actionDump_to_File.text(), "Save file error");
			}
		}
	});
	_menu.addAction(&actionLoad_From_File);
	connect(&actionLoad_From_File, &QAction::triggered, [=]
	{
		QString name = QFileDialog::getOpenFileName(this, tr("Save Current Image"), ".",
			tr(supportFileFormats));
		if (name.isEmpty() == false)
		{
			try
			{
				if (SetImg(name) == false)
					throw std::exception("Load file error");
			}
			catch (...)
			{
				QMessageBox::warning(this, actionLoad_From_File.text(), "Save file error");
			}
		}
			
	});

	this->setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
}

MatGraphicsview::~MatGraphicsview()
{
}

bool MatGraphicsview::SetImg(const cv::Mat&img)
{
	_image = img.clone();
	return _SetImgWithSettings();
}

bool MatGraphicsview::SetImg(const QString&filename)
{
	_image = cv::imread(filename.toStdString());
	if (_image.empty() == true)
		return false;
	return _SetImgWithSettings();
}

void MatGraphicsview::wheelEvent(QWheelEvent *event)
{
	QPoint scrollAmount = event->angleDelta();
	
	float  newScaleFactor = _setting.scaleFactor*((scrollAmount.y()>0)?(1.259921049894873164):(1/ 1.259921049894873164));//2^(1/3)=1.259921049894873164
	_Zoom(newScaleFactor);
	_setting.sizePolicy = S_Settings::E_SizePolicy::Free;
}

bool MatGraphicsview::_SetImgWithSettings()
{
	if (_image.empty() == true)
		return false;
// 	//单通道下，不允许Ignore Alpha,DrawContour,Channel
// 	if (_image.channels() == 1 || action_Channel_All.isChecked() == false
// 		|| action_Convert[_setting.Gray].isChecked() == false || actionDraw_Contour.isChecked() == false)
// 	{
// 		actionIgnore_Alpha.setEnabled(false);
// 		actionDraw_Contour.setEnabled(false);
// 		_menu_Channel.setEnabled(false);
// 		_menu_Convert.setEnabled(false);
// 	}
// 	else if (_image.channels()==2)
// 	{
// 		actionPseudo_Color.setEnabled(false);
// 	}

	//对比度拉伸，除非是需要画轮廓
	//执行完后，通道数不变（画轮廓则生成BGRA的二值图），但【深度为8】
	if(_image.channels() != 2|| _setting.isContour == false)//不需要画轮廓
	{
		if (_setting.isMaxContrast == true)//如果选中了“自动最大化对比度”，则进行转换
		{
			double minval = 0, maxVal = 0;
			minMaxLoc(_image, &minval, &maxVal);
			convertScaleAbs(_image, _procImg, 255.0 / (maxVal - minval), -minval * 255 / (maxVal - minval));
		}
		else 
		{
			_image.convertTo(_procImg, CV_8U);
			//convertScaleAbs(_image, _procImg, 1);//否则按照1:1转换为8位整数并做截断
		}
	}
	else//转换为轮廓图，黑底，白色轮廓，4通道
	{
		cv::Rect imgRect = cv::boundingRect(_image);
		_procImg.create(imgRect.size(), CV_8UC3);
		_procImg.setTo(cv::Scalar::all(0));
		cv::drawContours(_procImg, _image, -1, cv::Scalar::all(255), 3);
	}

	//色彩转换
	//执行后，1通道不变，2通道变为4通道，其余根据转换之后的目标格式而定
	if (_procImg.channels() == 1)
		;
	else if (_procImg.channels() == 2)//双通道，则分别转换为B,G通道并添加上为0的R通道和255的A通道组成BGRA
	{
		std::vector<cv::Mat>vm;
		cv::split(_procImg, vm);
		vm.push_back(cv::Mat(vm[0].size(), CV_8U, cv::Scalar(0)));
		vm.push_back(cv::Mat(vm[0].size(), CV_8U, cv::Scalar(255)));
		cv::merge(vm, _procImg);
	}
	else if (_procImg.channels() == 3)
	{
		switch (_setting.colorType)
		{
		case S_Settings::E_ColorType::Default:break;
		case S_Settings::E_ColorType::Gray:cvtColor(_procImg, _procImg, cv::COLOR_BGR2GRAY); break;
		case S_Settings::E_ColorType::RGB:cvtColor(_procImg, _procImg, cv::COLOR_BGR2RGB); break;
		case S_Settings::E_ColorType::RGBA:cvtColor(_procImg, _procImg, cv::COLOR_BGR2RGBA); break;
		//case S_Settings::E_ColorType::BGR: break;
		case S_Settings::E_ColorType::BGRA:cvtColor(_procImg, _procImg, cv::COLOR_BGR2BGRA); break;
		case S_Settings::E_ColorType::HSV:cvtColor(_procImg, _procImg, cv::COLOR_BGR2HSV); break;
		case S_Settings::E_ColorType::YUV:cvtColor(_procImg, _procImg, cv::COLOR_BGR2YUV); break;
		case S_Settings::E_ColorType::YCrCb:cvtColor(_procImg, _procImg, cv::COLOR_BGR2YCrCb); break;
		default:
			//QMessageBox::information(this, action_Convert->text() + " " + actionGroup_Convert.checkedAction()->text(), "Not Support yet!");
			break;
		}
	}
	else if (_image.channels() == 4)
	{
		if (_setting.isIgnoreAlpha == true) 
		{
			cvtColor(_procImg, _procImg, cv::COLOR_BGRA2BGR);
			switch (_setting.colorType)
			{
			case S_Settings::E_ColorType::Default:break;
			case S_Settings::E_ColorType::Gray:cvtColor(_procImg, _procImg, cv::COLOR_BGR2GRAY); break;
			case S_Settings::E_ColorType::RGB:cvtColor(_procImg, _procImg, cv::COLOR_BGR2RGB); break;
			case S_Settings::E_ColorType::RGBA:cvtColor(_procImg, _procImg, cv::COLOR_BGR2RGBA); break;
			case S_Settings::E_ColorType::BGR: break;
			case S_Settings::E_ColorType::BGRA:cvtColor(_procImg, _procImg, cv::COLOR_BGR2BGRA); break;
			case S_Settings::E_ColorType::HSV:cvtColor(_procImg, _procImg, cv::COLOR_BGR2HSV); break;
			case S_Settings::E_ColorType::YUV:cvtColor(_procImg, _procImg, cv::COLOR_BGR2YUV); break;
			case S_Settings::E_ColorType::YCrCb:cvtColor(_procImg, _procImg, cv::COLOR_BGR2YCrCb); break;
			default:
				QMessageBox::information(this, action_Convert->text() + " " + actionGroup_Convert.checkedAction()->text(), "Not Support yet!");
				break;
			}
		}
		switch (_setting.colorType)
		{
		case S_Settings::E_ColorType::Default:break;
		case S_Settings::E_ColorType::Gray:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2GRAY); break;
		case S_Settings::E_ColorType::RGB:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2RGB); break;
		case S_Settings::E_ColorType::RGBA:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2RGBA); break;
		case S_Settings::E_ColorType::BGR:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2BGR); break;
		case S_Settings::E_ColorType::HSV:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2BGR); cvtColor(_procImg, _procImg, cv::COLOR_BGR2HSV); break;
		case S_Settings::E_ColorType::YUV:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2BGR); cvtColor(_procImg, _procImg, cv::COLOR_BGR2YUV); break;
		case S_Settings::E_ColorType::YCrCb:cvtColor(_procImg, _procImg, cv::COLOR_BGRA2BGR); cvtColor(_procImg, _procImg, cv::COLOR_BGR2YCrCb); break;
		default:
			QMessageBox::information(this, action_Convert->text() + " " + actionGroup_Convert.checkedAction()->text(), "Not Support yet!");
			break;
		}
	}
	else
		return false;
	//通道选择
	//选中action_Channel_All则通道数不变，否则变为1
	if (_setting.channel != -1)//选择了单个通道
	{
		int ch = _setting.channel;
		if (ch <= _procImg.channels() - 1)
		{
			std::vector<cv::Mat>vm;
			split(_procImg, vm);
			_procImg = vm[ch];

			if (_setting.isMaxContrast == true)//如果选中了“自动最大化对比度”，则进行转换
			{
				double minval = 0, maxVal = 0;
				minMaxLoc(_procImg, &minval, &maxVal);
				convertScaleAbs(_procImg, _procImg, 255.0 / (maxVal - minval), -minval * 255 / (maxVal - minval));
			}
		}
		else
		{
			//action_Channel_All.setChecked(true);
		}
	}
	//伪彩色转换，转换为BGRA
	if (_procImg.type() == CV_8U && _setting.isPseudoColor == true)   //伪彩色
	{
		_procImg = PseudoTransform(_procImg);
	}

	//注意这里必须要进行复制，否则本函数结束后对应Mat tmp会被释放，使得其对应数据区data变成野指针
	//而从指针生成的QImage也不会进行复制，所以当重绘时候会调用到QImage对应的被Mat tmp释放的野指针造成崩溃
	switch (_procImg.channels())
	{
	case 1:	_backPixmap.setPixmap(QPixmap::fromImage(QImage(_procImg.data, _procImg.cols, _procImg.rows, QImage::Format_Grayscale8))); break;
	case 3:	
		cvtColor(_procImg, _procImg, cv::COLOR_BGR2BGRA);
		_backPixmap.setPixmap(QPixmap::fromImage(QImage(_procImg.data, _procImg.cols, _procImg.rows, QImage::Format_ARGB32_Premultiplied)));
		break;
	case 4:	_backPixmap.setPixmap(QPixmap::fromImage(QImage(_procImg.data, _procImg.cols, _procImg.rows, QImage::Format_ARGB32_Premultiplied))); break;
	default:
		break;
	}

// 	_backPixmap.setPixmap(QPixmap::fromImage(QImage(_procImg.data, _procImg.cols, _procImg.rows, QImage::Format_ARGB32))
// 		.copy(0, 0, _procImg.cols, _procImg.rows));

	if (_setting.sizePolicy == S_Settings::E_SizePolicy::Adapt)
		_ZoomToFit();

	emit sig_ImageSet(_image.type(), _image.channels(), QSize(_image.cols, _image.rows), _setting.scaleFactor, _setting.rotateAngle);
	return true;
}

void MatGraphicsview::_Zoom(float newscale)
{
	if (newscale < 0)
		return;
	if (newscale < 1.0 / 32)
		newscale = 1.0 / 32;
	if (newscale > 32)
		newscale = 32;
	scale(newscale / _setting.scaleFactor, newscale / _setting.scaleFactor);
	_setting.scaleFactor = newscale;
}

void MatGraphicsview::_ZoomToFit()
{
	_setting.sizePolicy = S_Settings::E_SizePolicy::Adapt;
	if (_backPixmap.pixmap().isNull() == true)
		return;
	auto widgetSize = this->viewport()->size();
	auto imgSize = _backPixmap.pixmap().size();
	float widthRatio = (float)widgetSize.width() / imgSize.width(), heightRatio = (float)widgetSize.height() / imgSize.height();

	if (widthRatio < heightRatio)
		_Zoom(widthRatio);
	else
		_Zoom(heightRatio);
}

void MatGraphicsview::_Rotate(float angle)
{

}

void MatGraphicsview::mouseDoubleClickEvent(QMouseEvent *event)
{
	_ZoomToFit();
	QGraphicsView::mouseDoubleClickEvent(event);
}

void MatGraphicsview::mouseMoveEvent(QMouseEvent *event)
{
	auto trans = transform().inverted();
	auto pt = trans.map(event->windowPos()-_backPixmap.pos());
	//qDebug()/*<<trans*/<<_backPixmap.scenePos() << _backPixmap.pos()<<event->localPos() <<event->windowPos() << event->pos() << _backPixmap.boundingRect();
	//emit sig_MouseTracking()
	
	QGraphicsView::mouseMoveEvent(event);
}

void MatGraphicsview::contextMenuEvent(QContextMenuEvent *event)
{
	_menu.exec(event->globalPos());
}

void MatGraphicsview::resizeEvent(QResizeEvent *event)
{
	if (_setting.sizePolicy == S_Settings::E_SizePolicy::Adapt)
		_ZoomToFit();
	QGraphicsView::resizeEvent(event);
}

static cv::Mat PseudoTransform(const cv::Mat&src)
{
	cv::Mat dst;
	cv::applyColorMap(src, dst, cv::COLORMAP_JET);
	return dst;
}