#pragma once

#include <QGraphicsScene>

class GraphScene : public QGraphicsScene
{
	Q_OBJECT

public:
	GraphScene(QWidget *parent);
	~GraphScene();
};
