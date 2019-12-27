#include "stdafx.h"
#include "CustomTypes.h"
#include "GraphError.h"

QString QVariant2Description(QVariant var, int len /*= 31*/)
{
	//TODO:之后可能会根据特定类型和不同长度给定制
	//例如QList<int>，如果小于某个值则返回"QList<int>"，否则可以返回"[1,2,3,...]"
	QString result;
	if (var.isNull() == true)
		result = QStringLiteral("[Null]");
	else if (var.canConvert<QString>() == true)
		result = var.toString();
	else if (var.canConvert<double>() == true)
		result = QString::number(var.toDouble());
	else
	{
		switch (var.type())
		{
		case QVariant::Invalid:
			result = QStringLiteral("[Invalid]");
			break;
			// 	case QVariant::Bool:
			// 		break;
			// 	case QVariant::Int:
			// 		break;
			// 	case QVariant::UInt:
			// 		break;
			// 	case QVariant::LongLong:
			// 		break;
			// 	case QVariant::ULongLong:
			// 		break;
			// 	case QVariant::Double:
			// 		break;
			// 	case QVariant::Char:
			// 		break;
			// 	case QVariant::Map:
			// 		break;
			// 	case QVariant::List:
			// 		break;
			// 	case QVariant::String:
			// 		break;
			// 	case QVariant::StringList:
			// 		break;
			// 	case QVariant::ByteArray:
			// 		break;
			// 	case QVariant::BitArray:
			// 		break;
			// 	case QVariant::Date:
			// 		break;
			// 	case QVariant::Time:
			// 		break;
			// 	case QVariant::DateTime:
			// 		break;
			// 	case QVariant::Url:
			// 		break;
			// 	case QVariant::Locale:
			// 		break;
			// 	case QVariant::Rect:
			// 		break;
			// 	case QVariant::RectF:
			// 		break;
			 	case QVariant::Size:
				{
					QSize sz = var.toSize();
					result = QString("(%1,%2)").arg(sz.height()).arg(sz.width());
					break;
				}
			// 	case QVariant::SizeF:
			// 		break;
			// 	case QVariant::Line:
			// 		break;
			// 	case QVariant::LineF:
			// 		break;
			// 	case QVariant::Point:
			// 		break;
			// 	case QVariant::PointF:
			// 		break;
			// 	case QVariant::RegExp:
			// 		break;
			// 	case QVariant::RegularExpression:
			// 		break;
			// 	case QVariant::Hash:
			// 		break;
			// 	case QVariant::EasingCurve:
			// 		break;
			// 	case QVariant::Uuid:
			// 		break;
			// 	case QVariant::ModelIndex:
			// 		break;
			// 	case QVariant::PersistentModelIndex:
			// 		break;
			// 	case QVariant::LastCoreType:
			// 		break;
			// 	case QVariant::Font:
			// 		break;
			// 	case QVariant::Pixmap:
			// 		break;
			// 	case QVariant::Brush:
			// 		break;
			// 	case QVariant::Color:
			// 		break;
			// 	case QVariant::Palette:
			// 		break;
			// 	case QVariant::Image:
			// 		break;
			// 	case QVariant::Polygon:
			// 		break;
			// 	case QVariant::Region:
			// 		break;
			// 	case QVariant::Bitmap:
			// 		break;
			// 	case QVariant::Cursor:
			// 		break;
			// 	case QVariant::KeySequence:
			// 		break;
			// 	case QVariant::Pen:
			// 		break;
			// 	case QVariant::TextLength:
			// 		break;
			// 	case QVariant::TextFormat:
			// 		break;
			// 	case QVariant::Matrix:
			// 		break;
			// 	case QVariant::Transform:
			// 		break;
			// 	case QVariant::Matrix4x4:
			// 		break;
			// 	case QVariant::Vector2D:
			// 		break;
			// 	case QVariant::Vector3D:
			// 		break;
			// 	case QVariant::Vector4D:
			// 		break;
			// 	case QVariant::Quaternion:
			// 		break;
			// 	case QVariant::PolygonF:
			// 		break;
			// 	case QVariant::Icon:
			// 		break;
			// 	case QVariant::SizePolicy:
			// 		break;
				case QVariant::UserType: 
				{
					if (var.canConvert<cv::Mat>() == true)
					{
						cv::Mat&m = var.value<cv::Mat>();
						result = QString("Mat {%1x%2x%3,%4}").arg(m.rows).arg(m.cols).arg(m.channels()).arg(MatType2Description(m.type()));
					}
					break;
				}
			// 	case QVariant::LastType:
			// 		break;
		default:
			result = QString(var.typeName());
			break;
		}
	}
	if (len > 0) 
	{
		if (result.size() > len)
		{
			result = result.left(len);
			result.back() = '~';
		}
	}
	return result;
}

QString MatType2Description(int type)
{
#define MatType2Description_TRANSFORM(tp) case tp:return QStringLiteral(#tp);
	switch (type)
	{
	MatType2Description_TRANSFORM(CV_8UC1 )
	MatType2Description_TRANSFORM(CV_8UC2 )
	MatType2Description_TRANSFORM(CV_8UC3 )
	MatType2Description_TRANSFORM(CV_8UC4 )

	MatType2Description_TRANSFORM(CV_8SC1 )
	MatType2Description_TRANSFORM(CV_8SC2 )
	MatType2Description_TRANSFORM(CV_8SC3 )
	MatType2Description_TRANSFORM(CV_8SC4 )

	MatType2Description_TRANSFORM(CV_16UC1 )
	MatType2Description_TRANSFORM(CV_16UC2 )
	MatType2Description_TRANSFORM(CV_16UC3 )
	MatType2Description_TRANSFORM(CV_16UC4 )

	MatType2Description_TRANSFORM(CV_16SC1 )
	MatType2Description_TRANSFORM(CV_16SC2 )
	MatType2Description_TRANSFORM(CV_16SC3 )
	MatType2Description_TRANSFORM(CV_16SC4 )

	MatType2Description_TRANSFORM(CV_32SC1 )
	MatType2Description_TRANSFORM(CV_32SC2 )
	MatType2Description_TRANSFORM(CV_32SC3 )
	MatType2Description_TRANSFORM(CV_32SC4 )

	MatType2Description_TRANSFORM(CV_32FC1 )
	MatType2Description_TRANSFORM(CV_32FC2 )
	MatType2Description_TRANSFORM(CV_32FC3 )
	MatType2Description_TRANSFORM(CV_32FC4 )

	MatType2Description_TRANSFORM(CV_64FC1 )
	MatType2Description_TRANSFORM(CV_64FC2 )
	MatType2Description_TRANSFORM(CV_64FC3 )
	MatType2Description_TRANSFORM(CV_64FC4 )

	MatType2Description_TRANSFORM(CV_16FC1 )
	MatType2Description_TRANSFORM(CV_16FC2 )
	MatType2Description_TRANSFORM(CV_16FC3 )
	MatType2Description_TRANSFORM(CV_16FC4 )
	default:return QStringLiteral("Unknown");
	}
#undef MatType2Description_TRANSFORM
}

QPixmap Mat2QPixmap(cv::Mat m)
{
	if (m.empty() == true)
		return QPixmap();
	else
	{
		switch (m.type())
		{
		case CV_8UC3:cvtColor(m, m, cv::COLOR_BGR2BGRA); break;
		case CV_8U:cvtColor(m, m, cv::COLOR_GRAY2BGRA); break;
		case CV_8UC4:break;
		default:
			assert(1);
			break;
		}
		//注意这里必须要进行复制，否则本函数结束后对应Mat tmp会被释放，使得其对应数据区data变成野指针
		//而从指针生成的QImage也不会进行复制，所以当重绘时候会调用到QImage对应的被Mat tmp释放的野指针造成崩溃
		return QPixmap::fromImage(QImage(m.data, m.cols, m.rows, QImage::Format_ARGB32))
			.copy(0, 0, m.cols, m.rows);
	}
}

QPixmap Mat2QPixmap(QVariant m)
{
	GRAPH_NOT_IMPLEMENT;
}
