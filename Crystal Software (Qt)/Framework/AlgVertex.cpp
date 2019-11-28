#include "stdafx.h"
#include "AlgVertex.h"
#include "AlgNode.h"
//#include "GuiVertex.h"

AlgVertex::AlgVertex(QWeakPointer<AlgNode>parent, VertexType type, QString name, Behavior_NoData defaultState,
	Behavior_BeforeActivate beforeBehavior, Behavior_AfterActivate afterBehavior, QVariant defaultData/* = QVariant()*/)
	:_node(parent),type(type),_behaviorNoData(defaultState),
	_behaviorBefore(beforeBehavior),_behaviorAfter(afterBehavior), _defaultData(defaultData)
{
	GRAPH_ASSERT(parent.isNull() == false);
	_amount++;
	SetSelfPointer();
#ifdef _DEBUG
	connect(this, &AlgNode::objectNameChanged, [this](QString str) {__debugname = str; });
#endif // _DEBUG
	setObjectName(name);
	inputAssertFunctions.append([this](const QVariant&)
	{return (_qdata.size() < AlgVertex_MaxBufferSize) ? true : false; });//最大BUFFER
}

QSharedPointer<AlgVertex> AlgVertex::Create(QWeakPointer<AlgNode>parent, VertexType type, QString name,
	Behavior_NoData defaultState, Behavior_BeforeActivate beforeBehavior,
	Behavior_AfterActivate afterBehavior, QVariant defaultData /*= QVariant()*/)
{
 	auto pvtx = QSharedPointer<AlgVertex>::create(parent, type, name, defaultState, beforeBehavior, afterBehavior, defaultData);
	pvtx->_weakRef = pvtx;
 	return pvtx;
}

bool AlgVertex::RemoveFromParent()
{
	if (_node.isNull() == false)
		return _node.lock()->RemoveVertex(type, objectName());
	else 
		return false;
}

AlgVertex::~AlgVertex()
{
	qDebug() << objectName() << __FUNCTION__;
	try
	{
		Clear();
		RemoveFromParent();
	}
	catch (...)//如果无法避免Release()抛出异常，至少要无条件吞掉异常
	{
		qDebug() << "Error in " __FUNCTION__;
		assert(0);
	}
	_amount--;
}

void AlgVertex::Activate(QVariant var)
{
	if (_isEnabled == true)
	{
		qDebug() << _node.lock()->objectName() + ':' + objectName() << __FUNCTION__;
		emit sig_ActivateBegin();
		for (auto &f : inputAssertFunctions)
			if (f(var) == false)
				return;//是否抛出异常由函数自己决定

		_qdata.push_back(var);
		emit sig_Activated(_qdata.front()/*, isNow*/);
		emit sig_ActivateEnd();
// 		if (isActivate == true)
// 		{
// 			_qdata.push_back(var);
// 			while (_behaviorBefore == Behavior_BeforeActivate::DIRECT && _qdata.size() > 1)//DIRECT只保留一个
// 				_qdata.pop_front();
// 		}
// 		else
// 			_qdata.clear();//TODO:这里要考虑一下是全部清除还是只清除一部分
// 
// 		_isActivated = isActivate;
// 		emit sig_Activated(var, isActivate);
// 		emit sig_ActivateEnd();
	}
}

void AlgVertex::slot_ActivateSuccess()
{
	if (_qdata.empty() == false) 
	{
		_lastdata = _qdata.front();
		_qdata.pop_front();
	}
// 	if (_qdata.empty() == true)//队列空
// 	{
// 		if (_behaviorAfter == Behavior_AfterActivate::RESET)
// 			_isActivated = false;
// 	}
// 	else//如果队列非空，则继续激活（此时必定_isActivated==true，因为如果Activate()中是false则队列已被清空）
// 	{
// 		_qdata.pop_front();
// 		//TODO:有个重大问题，如果此时NODE所有节点均为激活，这样做会多次激活？或许还是由Node主动调用比较好
// 		
// // 		emit sig_ActivateBegin();
// // 		emit sig_Activated(_qdata.front(), _isActivated);
// // 		emit sig_ActivateEnd();
// 	}
}

void AlgVertex::Connect(QSharedPointer<AlgVertex>dstVertex)
{
	GRAPH_ASSERT(dstVertex.isNull() == false);
	for (auto &f : connectAssertFunctions)
		GRAPH_ASSERT(f(dstVertex) == true);
	_nextVertexes.append(dstVertex->WeakRef());
	dstVertex->_prevVertexes.append(WeakRef());

	if (type == AlgVertex::VertexType::INPUT)
		connect(this, &AlgVertex::sig_Activated, dstVertex.data(), &AlgVertex::Activate, Qt::QueuedConnection);//必须改成QueuedConnection，否则是直连，相当于会递归调用进行深度优先遍历
	else
		connect(this, &AlgVertex::sig_Activated, dstVertex.data(), &AlgVertex::Activate, Qt::DirectConnection);

	emit sig_ConnectionAdded(StrongRef(), dstVertex);
}

void AlgVertex::Disconnect(QWeakPointer<AlgVertex>another)
{
	if (_nextVertexes.removeAll(another) > 0)//自身是起点
	{
		if (another.isNull() == false) 
		{
			QSharedPointer<AlgVertex>spanother = another.lock();
			spanother->_prevVertexes.removeAll(WeakRef());//这里不能用shareFromThis(),因为析构时候因为sharepointer已经消失，将返回nullptr
			qDebug() << objectName() + '-' + spanother->objectName() << __FUNCTION__;
			disconnect(this, &AlgVertex::sig_Activated, spanother.data(), &AlgVertex::Activate);
			emit sig_ConnectionRemoved(WeakRef().data(), another.data());
		}
	}
	else if (_prevVertexes.removeAll(another) > 0) //自身是终点
	{
		if (another.isNull() == false) 
		{
			//another->Disconnect(WeakRef().lock());//不能这么用，因为析构时候已被删除
			QSharedPointer<AlgVertex>spanother = another.lock();
			spanother->_nextVertexes.removeAll(WeakRef());
			qDebug() << spanother->objectName() + '-' + objectName() << __FUNCTION__;
			disconnect(spanother.data(), &AlgVertex::sig_Activated, this, &AlgVertex::Activate);
			emit spanother->sig_ConnectionRemoved(another.data(), WeakRef().data());
		}
	}
}

void AlgVertex::Reset()
{
	_qdata.clear();
	if (_behaviorNoData == Behavior_NoData::USE_LAST_FIRST_DEFAULT
		| _behaviorNoData == Behavior_NoData::USE_NULL_FIRST_DEFAULT)
		_lastdata = _defaultData;
	else
		_lastdata.clear();
}

void AlgVertex::Clear()
{
	Reset();
	_isEnabled = true;
	while (_nextVertexes.size() > 0)
		Disconnect(_nextVertexes.back());
	while (_prevVertexes.size() > 0)
		Disconnect(_prevVertexes.back());
}


QVariant AlgVertex::GetData() const
{
	if (_qdata.size() > 0)
		return _qdata.front();
	switch (_behaviorNoData)
	{
	case Behavior_NoData::USE_NULL_FIRST_DEFAULT:
	case AlgVertex::Behavior_NoData::USE_NULL:
		return QVariant();
	case Behavior_NoData::USE_LAST_FIRST_DEFAULT:
	case AlgVertex::Behavior_NoData::USE_LAST:
		return _lastdata;
	case AlgVertex::Behavior_NoData::USE_DEFAULT:
		return _defaultData;
	default:
		GRAPH_NOT_IMPLEMENT;
	}
	//_qdata.size() > 0 ? _qdata.front() : _defaultData;
}

std::atomic_uint64_t AlgVertex::_amount;
