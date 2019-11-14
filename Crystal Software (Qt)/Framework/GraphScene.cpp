#include "stdafx.h"
#include "GraphScene.h"

GraphScene::GraphScene(QWidget *parent)
	: QGraphicsScene(parent)
{
}

GraphScene::~GraphScene()
{
	qDebug() << objectName() << __FUNCTION__;
}
