#include "stdafx.h"
#include "BRISQUE_Assess.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

QPixmap Mat2QPixmap(cv::Mat m);

BRISQUE_Assess::BRISQUE_Assess(QString setting, QString input, QString output, QWidget *parent)
	: QMainWindow(parent),_inputDir(input),_outputFile(output)
{
	ui.setupUi(this);
	
	if (_inputDir.isEmpty() == true)
		_inputDir = QApplication::applicationDirPath();
	QDir dir(_inputDir);
	if (output.isEmpty() == true)
		_outputFile = dir.absoluteFilePath("dmos.txt");

	//��ȡ�����ļ�����
	QFile oldOutputFile(_outputFile);
	if (oldOutputFile.exists() == true && oldOutputFile.size() > 0
		&& oldOutputFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream ts(&oldOutputFile);
		while (ts.atEnd() == false)
		{
			auto words = ts.readLine().split(',');
			if (words.size() == 0)continue;
			for (auto i = 1; i < words.size(); i++)
			{
				float f; bool b;
				f = words[i].toFloat(&b);
				if (b == true)
					_oldDmosTbl[words[0]].append(f);
			}
		}
	}
	else
		_outputFile = dir.absoluteFilePath("dmos.txt");
	setWindowTitle(QStringLiteral("Դ�ļ��У�%0  Ŀ�꣺%1").arg(_inputDir).arg(_outputFile));

	//��ȡ	
	QStringList result = dir.entryList(
		QStringList{ "*.png","*.jpg","*.jpeg","*.bmp", },
		QDir::Files, QDir::SortFlag::Name);
	for (auto r : result) 
	{
		if (_oldDmosTbl.contains(dir.absoluteFilePath(r)) == false)
			_oldDmosTbl.insert(r, QList<float>{});
	}
	_dmosTbl = _oldDmosTbl;
	_filenames = _oldDmosTbl.keys();
	std::random_shuffle(_filenames.begin(), _filenames.end());
	_nextIdx = 0;
	
	statusBar()->showMessage(QStringLiteral("������ɣ���[%0]��").arg(_dmosTbl.size()));

	connect(ui.verticalSlider_Left, &QSlider::valueChanged, ui.label_3,QOverload<int>::of(&QLabel::setNum));
	connect(ui.verticalSlider_Left, &QSlider::valueChanged, [this] {_isClick[0] = true; });
	connect(ui.verticalSlider_Right, &QSlider::valueChanged, ui.label_4, QOverload<int>::of(&QLabel::setNum));
	connect(ui.verticalSlider_Right, &QSlider::valueChanged, [this] {_isClick[1] = true; });

	connect(ui.pushButton, &QPushButton::clicked, this,&BRISQUE_Assess::Next);
	connect(ui.action_Save, &QAction::triggered, [this] {this->Save(false); });
	connect(ui.action_Recover, &QAction::triggered, [this] {_dmosTbl = _oldDmosTbl; _nextIdx = 0; Next(); });

	startTimer(60000, Qt::TimerType::VeryCoarseTimer);

	Next();
}

BRISQUE_Assess::~BRISQUE_Assess()
{
}

// void BRISQUE_Assess::Reset()
// {
// 	ui.verticalSlider_Left->setValue(0);
// 	ui.verticalSlider_Right->setValue(0);
// }

void BRISQUE_Assess::Next()
{
	if (_filenames.empty() == true || _filenames.size() != _dmosTbl.size())
		return;

	if (_nextIdx > 0)//�ǵ�һ�ε��ã���Ҫ����
	{
		if (_isClick[0] == true)
			_dmosTbl[_filenames[_nextIdx - 2]]
			.append(float(ui.verticalSlider_Left->value()) / ui.verticalSlider_Left->maximum());
		if (_isClick[1] == true)
			_dmosTbl[_filenames[_nextIdx - 1]]
			.append(float(ui.verticalSlider_Right->value()) / ui.verticalSlider_Right->maximum());
		if (_nextIdx % 10 < 1)
			Save(true);
	}
	else
		_nextIdx = 0;

	ui.verticalSlider_Left->setValue(0);
	_isClick[0] = false;
	ui.verticalSlider_Right->setValue(0);
	_isClick[1] = false;

	QDir dir(_inputDir);
	QString leftName = dir.absoluteFilePath(_filenames[(_nextIdx++) % _filenames.size()]),
		rightName = dir.absoluteFilePath(_filenames[(_nextIdx++) % _filenames.size()]);

	ui.label_Left->setPixmap(Mat2QPixmap(cv::imread(leftName.toStdString(),cv::IMREAD_GRAYSCALE)));
	//ui.label_Left->setProperty("path", leftName);
	ui.label_Left->setToolTip(leftName);

	ui.label_Right->setPixmap(Mat2QPixmap(cv::imread(rightName.toStdString(), cv::IMREAD_GRAYSCALE)));
	//ui.label_Right->setProperty("path", rightName);
	ui.label_Right->setToolTip(rightName);

	statusBar()->showMessage(QStringLiteral("%0/%1").arg(_nextIdx).arg(_dmosTbl.size()));
}

void BRISQUE_Assess::Save(bool isTemp /*= true*/)
{
	QDir dir(_inputDir);
	QFile f(_outputFile+".tmp", this);
	if (f.open(QIODevice::WriteOnly | QIODevice::Text)==true)
	{
		QTextStream ts(&f);
		for (auto it = _dmosTbl.cbegin(); it != _dmosTbl.cend(); ++it)
		{
			ts << dir.absoluteFilePath(it.key()) << ',';
			QStringList words;
			for (auto num : *it)
				words.append(QString::number(num));
			ts << words.join(',') << endl;
		}
		f.close();
		if (isTemp == false)
		{
			QFile::remove(_outputFile);
			f.rename(_outputFile);			
		}
		statusBar()->showMessage(QStringLiteral("���浽%0").arg(f.fileName()), 1000);
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("�������"),
			QStringLiteral("�޷��򿪣�") + f.fileName());
		_outputFile = QFileDialog::getSaveFileName(this, "", _inputDir);
	}
}

void BRISQUE_Assess::closeEvent(QCloseEvent *event)
{
	auto choice = QMessageBox::information(this, QStringLiteral("�Ƿ񱣴�"),
		QStringLiteral("�Ƿ񱣴浽��") + _outputFile, QStringLiteral("����"), QStringLiteral("������"));
	if (choice == 0)
		Save(false);
}

void BRISQUE_Assess::timerEvent(QTimerEvent *event)
{
	Save(true);
}
QPixmap Mat2QPixmap(cv::Mat m)
{
	if (m.empty() == true)
		return QPixmap();
	else
	{
		switch (m.type())
		{
		case CV_8UC3:cvtColor(m, m, cv::COLOR_BGR2BGRA); break;
		case CV_8U:cvtColor(m, m, cv::COLOR_GRAY2BGRA); break;
		case CV_8UC4:break;
		default:
			assert(1);
			break;
		}
		//ע���������Ҫ���и��ƣ����򱾺����������ӦMat tmp�ᱻ�ͷţ�ʹ�����Ӧ������data���Ұָ��
		//����ָ�����ɵ�QImageҲ������и��ƣ����Ե��ػ�ʱ�����õ�QImage��Ӧ�ı�Mat tmp�ͷŵ�Ұָ����ɱ���
		return QPixmap::fromImage(QImage(m.data, m.cols, m.rows, QImage::Format_ARGB32))
			.copy(0, 0, m.cols, m.rows);
	}
}