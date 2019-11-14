#pragma once
#include "global.h"
#include <QGraphicsScene>

class GraphScene : public QGraphicsScene
{
	Q_OBJECT

public:
	GraphScene(QWidget *parent);
	~GraphScene();

	virtual void keyPressEvent(QKeyEvent *event) override;

signals:
	void sig_RemoveItems(QList<QGraphicsItem*>items);
};
