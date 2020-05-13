#pragma once

#include <QMainWindow>
#include "ui_BRISQUE_Assess.h"

//ֱ��ʹ�û���Ԥ����ͷ����ı��������Դ�ļ���û���ҵ��� /Ycstdafx.h ������ѡ��ָ���ġ�#include�����
//���������ͷ�ļ��Ҽ�-����-moc-prepend files����stdafx.h

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
