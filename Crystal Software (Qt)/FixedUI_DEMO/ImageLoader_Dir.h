#pragma once

#include <QDockWidget>
#include "ui_ImageLoader_Dir.h"
#include "Interface_ImageLoader.h"

class ImageLoader_Dir : public QDockWidget,public Interface_ImageLoader
{
	Q_OBJECT

public:
	ImageLoader_Dir(QWidget *parent = Q_NULLPTR);
	~ImageLoader_Dir();


	virtual bool Load(QStringList path) override;
	virtual void Clear() override;


	virtual QVariant Get(QVariantHash*extraInfo = nullptr) override;
	virtual bool Seek(size_t idx) override;


	virtual int size() const override;
	virtual int Pos() const override;
	virtual bool IsOpen() const override;
	virtual bool IsEnd() const override;


	virtual QVariantHash SetSetting(QVariantHash setting) override;
	virtual QVariantHash GetSetting() const override;


	//virtual QVariantList Get(size_t begin, size_t end = UINT_MAX) override;

private:
	enum {
		COLUMN_STATUS=0,
		COLUMN_FILENAME=1,
		COLUMN_DIR=2
	};//各个列对应下标
	Ui::ImageLoader_Dir ui;
	int _ptr = 0;
};
