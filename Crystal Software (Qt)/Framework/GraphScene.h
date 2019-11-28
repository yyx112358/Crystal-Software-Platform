#pragma once
#include "global.h"
#include <QGraphicsScene>

class GuiConnection;
class GuiVertex;

class GraphScene : public QGraphicsScene
{
	Q_OBJECT

public:
	GraphScene(QWidget *parent);
	~GraphScene();

	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;//主要处理画线
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
signals:
	void sig_ConnectionAdded(QSharedPointer<GuiVertex> src, QSharedPointer<GuiVertex> dst);
	void sig_RemoveItems(QList<QGraphicsItem*>items);
protected:
	//QSharedPointer<GuiConnection>arrow;
	QGraphicsLineItem arrow;
};
