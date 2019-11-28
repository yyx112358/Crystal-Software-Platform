#include "stdafx.h"
#include "GraphScene.h"
#include "AlgVertex.h"
#include "GuiNode.h"
#include "GuiVertex.h"
#include "GuiConnection.h"

GraphScene::GraphScene(QWidget *parent)
	: QGraphicsScene(parent), arrow(QLineF(0, 0, 0, 0), nullptr)
{
	arrow.setVisible(false);
	arrow.setZValue(0.3);
	addItem(&arrow);
}

GraphScene::~GraphScene()
{
	qDebug() << objectName() << __FUNCTION__;
	//arrow.clear();
	removeItem(&arrow);
}

void GraphScene::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Delete:
		emit sig_RemoveItems(selectedItems());
		break;
	default:
		break;
	}
}
void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)//主要处理画线
{
	if (event->button() == Qt::LeftButton)
	{
		if (arrow.isVisible() == false)
		{
			auto item = itemAt(event->scenePos(), QTransform());
			auto vtxItem = qgraphicsitem_cast<GuiVertex*>(item);
			if (vtxItem != nullptr)
			{
				auto pos = item->mapToScene(item->boundingRect().center());
				//arrow = GuiConnection::Create(vtxItem->StrongRef()->algVertex.lock().constCast<AlgVertex>(),nullptr);
				//arrow->SetLine(QLineF(arrow->Line().p1(), pos));
				//addItem(arrow.data());
				arrow.setLine(QLineF(pos, pos));
				arrow.setVisible(true);
			}
		}
		else
		{
// 			auto srcPos = arrow->Line().p1(), dstPos = arrow->Line().p2();
// 			arrow.clear();
			auto srcPos = arrow.line().p1(), dstPos = arrow.line().p2();
			arrow.setVisible(false);

			auto dstItem = qgraphicsitem_cast<GuiVertex*>(itemAt(dstPos, QTransform())),
				srcItem = qgraphicsitem_cast<GuiVertex*>(itemAt(srcPos, QTransform()));//必须先删掉arrow，否则获取的是arrow
			if (dstItem != nullptr&&srcItem != nullptr && srcItem->type() == GuiVertex::Type)
				emit sig_ConnectionAdded(srcItem->sharedFromThis(), dstItem->sharedFromThis());
		}
	}

	QGraphicsScene::mousePressEvent(event);
}
void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (arrow.isVisible() == true)
	{
		auto item = itemAt(event->scenePos(), QTransform());
		if (qgraphicsitem_cast<GuiVertex*>(item) != nullptr)
		{
			auto pos = item->mapToScene(item->boundingRect().center());
			arrow.setLine(QLineF(arrow.line().p1(), pos));
		}
		else
			arrow.setLine(QLineF(arrow.line().p1(), event->scenePos()));
	}
	QGraphicsScene::mouseMoveEvent(event);
}