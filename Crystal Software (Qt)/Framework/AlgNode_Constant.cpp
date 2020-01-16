#include "stdafx.h"
#include "AlgNode_Constant.h"

AlgNode_Constant::AlgNode_Constant(QThreadPool&pool /*= *QThreadPool::globalInstance()*/, QObject*parent /*= nullptr*/)
{
	_mode = RunMode::Direct;
	_isUnchange = true;
#ifdef _DEBUG
	_data = 1.5;
#endif // _DEBUG
}

QVariantHash AlgNode_Constant::_Run(QVariantHash data)
{
	return QVariantHash({ std::pair<QString, QVariant>("  ", _data) });
}

void AlgNode_Constant::Init()
{
	_isUnchange = false;
	AddVertex(AlgVertex::VertexType::OUTPUT, "  ", AlgVertex::Behavior_NoData::USE_LAST_FIRST_DEFAULT,
		AlgVertex::Behavior_BeforeActivate::DIRECT, AlgVertex::Behavior_AfterActivate::KEEP, QVariant());//默认名称为2个空格
	_isUnchange = true;
}

void AlgNode_Constant::ProcessAction(QString action, bool isChecked)
{
	if (action == "edit")
	{
		bool ok = false;
		double result = _data.toDouble(&ok);
		if (ok == false)
			return;
		result = QInputDialog::getDouble(nullptr, "Edit value", "Input value", result, INT_MIN, INT_MAX, 3, &ok);
		if (ok) 
		{
			_data = result;
			emit sig_DataChanged(_data);
		}
	}
	else
		AlgNode::ProcessAction(action, isChecked);
}
