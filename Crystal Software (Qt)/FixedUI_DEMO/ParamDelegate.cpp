#include "stdafx.h"
#include "ParamDelegate.h"
#include "CustomTypes.h"
#include <opencv2/imgproc.hpp>
#include "MatGraphicsView.h"

ParamDelegate::ParamDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

ParamDelegate::~ParamDelegate()
{
}

void ParamDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem myoption = option;
	if (index.data().canConvert<cv::Mat>() == true)
	{
		cv::Mat m = index.data(Qt::EditRole).value<cv::Mat>();

		if (myoption.state & QStyle::State_Selected)
			painter->fillRect(myoption.rect, myoption.palette.highlight());		
		
		cv::resize(m, m, cv::Size(50, 50));//TODO:等比例缩放至最大50*50
		QPixmap pix = Mat2QPixmap(m);
		painter->drawPixmap(myoption.rect.x(), myoption.rect.y(), pix);
		myoption.rect.setX(myoption.rect.x() + pix.width());

		QString str = displayText(index.data(), QLocale::system());
		painter->drawText(myoption.rect, str);
	}
	else
		QStyledItemDelegate::paint(painter, option, index);
}

QSize ParamDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data().canConvert<cv::Mat>() == true)
	{
		auto spacing = option.font.pointSize() * sizeof("cv::Mat of[1000 x 1000 x 1000]");
		return QSize(50 + spacing, 50);
	}
	else
		return QStyledItemDelegate::sizeHint(option, index);
}

QWidget * ParamDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data().canConvert<cv::Mat>() == true)
		return new MatGraphicsview(parent);
	else
		return QStyledItemDelegate::createEditor(parent, option, index);
}

void ParamDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (index.data().canConvert<cv::Mat>() == true)
	{
		MatGraphicsview*myeditor = qobject_cast<MatGraphicsview*>(editor);
		if (myeditor == nullptr)
			return;
		myeditor->SetImg(index.data().value<cv::Mat>());
	}
	else
		return QStyledItemDelegate::setEditorData(editor, index);
}

void ParamDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if (index.data().canConvert<cv::Mat>() == true)
	{
		//TODO:
	}
	else
		return QStyledItemDelegate::setModelData(editor, model, index);
}

QString ParamDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	if (value.type() == QVariant::PointF)
	{
		auto dat = value.toPointF();
		return QString("[%1,%2]").arg(dat.rx()).arg(dat.ry());
	}
	else if (value.canConvert<cv::Mat>() == true)
	{
		cv::Mat m = value.value<cv::Mat>();
		return QString("cv::Mat of [%1x%2x%3]").arg(m.rows).arg(m.cols).arg(m.channels());
	}
	else
		return QStyledItemDelegate::displayText(value, locale);
}
