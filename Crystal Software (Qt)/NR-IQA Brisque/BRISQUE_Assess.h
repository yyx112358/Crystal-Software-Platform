#pragma once

#include <QMainWindow>
#include "ui_BRISQUE_Assess.h"

//直接使用会有预编译头引起的编译错误：在源文件中没有找到用 /Ycstdafx.h 命令行选项指定的“#include”语句
//解决方法：头文件右键-属性-moc-prepend files填上stdafx.h

class BRISQUE_Assess : public QMainWindow
{
	Q_OBJECT

public:
	BRISQUE_Assess(QString setting,QString input,QString output,QWidget *parent = Q_NULLPTR);
	~BRISQUE_Assess();

	void Next();
	void Save(bool isTemp = true);

protected:
	virtual void closeEvent(QCloseEvent *event) override;


	virtual void timerEvent(QTimerEvent *event) override;

private:
	Ui::BRISQUE_Assess ui;

	QString _inputDir, _outputFile;
	QMap<QString, QList<float>>_dmosTbl,_oldDmosTbl;
	QList<QString>_filenames;
	int _nextIdx = 0;
	bool _isClick[2] = { false,false };

};
