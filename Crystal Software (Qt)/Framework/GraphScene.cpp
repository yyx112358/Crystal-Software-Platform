#include "stdafx.h"
#include "GraphScene.h"
#include "GuiNode.h"

GraphScene::GraphScene(QWidget *parent)
	: QGraphicsScene(parent)
{
}

GraphScene::~GraphScene()
{
	qDebug() << objectName() << __FUNCTION__;
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
