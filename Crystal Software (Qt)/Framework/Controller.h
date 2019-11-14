#pragma once
#include "global.h"
#include <QMainWindow>
#include "ui_Controller.h"
#include "AlgNode.h"
#include "GraphScene.h"
#include "Factory_Basic.h"

class FRAMEWORK_EXPORT Controller : public QMainWindow
{
	Q_OBJECT

public:
	Controller(QWidget *parent = Q_NULLPTR);
	~Controller();
	void Release();

	void LoadFactory();
private:
	Ui::Controller ui;

	Factory_Basic _factory;

	GraphScene _scene;
	//QHash<QString, QWeakPointer<AlgNode>>_nodeSearchTbl;//���ұ�
	QList<QSharedPointer<AlgNode>>_nodes;
	QList<QWeakPointer<AlgNode>>_resumeNodes;//TODO:������֮ͣ�������

	void slot_CreateNode();
	void slot_Start();
	void slot_Pause(bool isPause);
	void slot_Stop();

	int _monitorTimerId = 0, _refreshTimerId = 1;
	virtual void timerEvent(QTimerEvent *event) final;

	QThreadPool _pool;
};
