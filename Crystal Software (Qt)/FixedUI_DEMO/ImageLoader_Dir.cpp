#include "stdafx.h"
#include "ImageLoader_Dir.h"
#include <QVariant>
#include <QFileDialog>
#include "GraphError.h"
#include "CustomTypes.h"
#include <opencv2/imgcodecs.hpp>
#include "MatGraphicsView.h"

ImageLoader_Dir::ImageLoader_Dir(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);

	auto _model = ui.tableWidget->model();
// 	ui.tableWidget->setColumnCount(3);
// 	_model->setHeaderData(0, Qt::Orientation::Horizontal, "", Qt::ItemDataRole::EditRole);
// 	_model->setHeaderData(1, Qt::Orientation::Horizontal, "Name", Qt::ItemDataRole::EditRole);
// 	_model->setHeaderData(2, Qt::Orientation::Horizontal, "Dir", Qt::ItemDataRole::EditRole);
	
	//ui.tableWidget->setModel(&_model);
	ui.tableWidget->setColumnWidth(0, 30);
	ui.tableWidget->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);//一次选中一行
	ui.tableWidget->setEditTriggers(QTableView::EditTrigger::NoEditTriggers);//不可修改
	ui.tableWidget->resizeColumnsToContents();//列宽随内容变化
	ui.tableWidget->setStyleSheet("selection-background-color:lightblue;");

	connect(ui.toolButton_Add, &QToolButton::clicked, [this]
	{
		QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open",
			".\\", "Supported (*.jpg *.jpeg *.png *.bmp)");
		Load(files);
	});
	connect(ui.toolButton_Reset, &QToolButton::clicked, [this]
	{
		Clear();
	});
	connect(ui.tableWidget, &QTableWidget::doubleClicked, [this](const QModelIndex&idx)
	{		
		int row = idx.row();

		auto path = GetPath(row);
		if (path.isEmpty()==false)
		{
			cv::Mat img = cv::imread(path.toStdString());
			if (img.empty() == false)
			{
				if (_viewer == nullptr)
					_viewer.reset(new MatGraphicsview(nullptr));
				_viewer->SetImg(img);
				_viewer->show();
			}
		}		
	});
}

ImageLoader_Dir::~ImageLoader_Dir()
{
}

bool ImageLoader_Dir::Load(QStringList paths)
{
	ui.tableWidget->setSortingEnabled(false);
	int begin = ui.tableWidget->rowCount(), end = begin + paths.size();
	ui.tableWidget->setRowCount(end);
	int row = begin;
	for (auto path : paths)
	{
		QFileInfo info(path);

		ui.tableWidget->setItem(row, COLUMN_STATUS, new QTableWidgetItem(""));
		ui.tableWidget->setItem(row, COLUMN_FILENAME, new QTableWidgetItem(info.fileName()));
		ui.tableWidget->setItem(row, COLUMN_DIR, new QTableWidgetItem(info.dir().absolutePath()));
		row++;
	}
	ui.tableWidget->setSortingEnabled(true);
	ui.tableWidget->resizeColumnsToContents();//列宽随内容变化
	return true;
}

void ImageLoader_Dir::Clear()
{
	//ui.tableWidget->clear();
	//ui.tableWidget->clearContents();
	ui.tableWidget->setRowCount(0);
	_ptr = 0;
}

QVariant ImageLoader_Dir::Get(QVariantHash*extraInfo /*= nullptr*/)
{
	QVariant result;
	if (IsOpen() == false || IsEnd() == true)
		return result;

	auto path = GetPath(_ptr);
	if (path.isEmpty()==false)
	{		
		cv::Mat img = cv::imread(path.toStdString());
		if (img.empty() == false) 
		{
			result = QVariant::fromValue<cv::Mat>(img);
			if (extraInfo != nullptr)
			{
				extraInfo->clear();
				extraInfo->insert("row", _ptr);
				extraInfo->insert("path", path);
			}
		}
	}
	if (result.isNull() == false) 
	{
		ui.tableWidget->item(_ptr, COLUMN_STATUS)->setBackgroundColor(Qt::green);
		ui.tableWidget->item(_ptr, COLUMN_STATUS)->setCheckState(Qt::PartiallyChecked);
	}
	else 
	{
		ui.tableWidget->item(_ptr, COLUMN_STATUS)->setBackgroundColor(Qt::red);
		ui.tableWidget->item(_ptr, COLUMN_STATUS)->setCheckState(Qt::Unchecked);
	}

	_ptr++;
	return result;
}


// QVariantList ImageLoader_Dir::Get(size_t begin, size_t end /*= UINT_MAX*/)
// {
// 	QVariantList qlist;
// 	if (end <= begin || begin>=size())
// 		return qlist;
// 	if (end > size())
// 		end = size();
// 	if (Seek(begin) == false)
// 		return qlist;
// 	for (auto i = 0; i < end; i++)
// 		qlist.append(Get(nullptr));
// 	return qlist;
// }

bool ImageLoader_Dir::Seek(size_t idx)
{
	if (idx < size())
	{
		_ptr = idx;
		return true;
	}
	else
		return false;
}

QString ImageLoader_Dir::GetPath(size_t idx, bool isCheck/*=true*/) const
{
	auto filename = ui.tableWidget->item(idx, COLUMN_FILENAME)->text(),
		dir = ui.tableWidget->item(idx, COLUMN_DIR)->text();
	QFileInfo info(dir, filename);
	if (isCheck == false
		|| (info.exists() == true && info.isFile() == true && info.size() > 0))
		return info.absoluteFilePath();
	else
		return QString();
}

int ImageLoader_Dir::size() const
{
	return ui.tableWidget->rowCount();
}

int ImageLoader_Dir::Pos() const
{
	return _ptr;
}

bool ImageLoader_Dir::IsOpen() const
{
	return true;
}

bool ImageLoader_Dir::IsEnd() const
{
	return _ptr >= size();
}

QVariantHash ImageLoader_Dir::SetSetting(QVariantHash setting)
{
	throw std::logic_error("The method or operation is not implemented.");
}

QVariantHash ImageLoader_Dir::GetSetting() const
{
	throw std::logic_error("The method or operation is not implemented.");
}
