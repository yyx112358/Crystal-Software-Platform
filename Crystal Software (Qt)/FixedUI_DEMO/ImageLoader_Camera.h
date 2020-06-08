#pragma once

#include <QDockWidget>
#include "ui_ImageLoader_Camera.h"
#include "Interface_ImageLoader.h"

class ImageLoader_Camera : public QDockWidget, public Interface_ImageLoader
{
	Q_OBJECT

public:
	ImageLoader_Camera(QWidget *parent = Q_NULLPTR);
	~ImageLoader_Camera();

private:
	Ui::ImageLoader_Camera ui;
};
