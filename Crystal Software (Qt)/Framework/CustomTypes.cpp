#include "stdafx.h"
#include "CustomTypes.h"


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
			// 	case QVariant::Size:
			// 		break;
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
			// 	case QVariant::UserType:
			// 		break;
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
