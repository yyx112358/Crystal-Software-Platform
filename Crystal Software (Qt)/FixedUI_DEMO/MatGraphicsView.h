#pragma once
/*!
 * \file MatGraphicsView.h
 * \date 2018/08/23 21:17
 *
 * \author yyx11
 *
 * \brief �Զ���QGraphicsView�����ڲ鿴Mat
 *	
 *
 * TODO: ����ƶ�ʱ��ȡ����ֵ
 *
 * \note
	�ο�image watch��֧�����ţ��������š�����Ӧ���š�ԭͼ��С�����϶�����ת���Զ��Աȶȣ�
 *	1ͨ��α��ɫ��4ͨ������alpha����дͼ�񣬻�������
*/
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QAction>
#include <QMenu>
#include <opencv2\core\mat.hpp>

class MatGraphicsview : public QGraphicsView
{
	Q_OBJECT

public:
	MatGraphicsview(QWidget *parent);
	~MatGraphicsview();

	bool SetImg(const cv::Mat&img);
	bool SetImg(const QString&filename);
	
	float ScaleFactor()const { return _setting.scaleFactor; }

	const static char supportFileFormats[];

signals:
	void sig_ImageSet(int type, int channel, QSize size, float scaleFactor, float rotateAngle);
	void sig_MouseTracking(QPoint p, cv::Vec4d color);
protected:
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;

	bool _SetImgWithSettings();

	void _Zoom(float scale);
	void _ZoomToFit();
	void _Rotate(float angle);//˳ʱ����תangle��

	QGraphicsScene _scene;
	QGraphicsPixmapItem _backPixmap;
	cv::Mat _image,_procImg;

	struct S_Settings
	{
		float scaleFactor = 1;//���ű���
		float rotateAngle = 0;//��ת�Ƕ�
		int channel = -1;//-1��ʾȫѡ

		enum class E_SizePolicy :unsigned char
		{
			Free,//��������
			Origin,//ԭʼ��С
			Adapt,//����Ӧ
		}sizePolicy = E_SizePolicy::Adapt;

		enum E_ColorType
		{
			Default = 0, Gray = 1, RGB = 2, RGBA = 3, BGR = 4,
			BGRA = 5, HSV = 6, YUV = 7, YCrCb = 8,
		}colorType = Default;

		const char *colorTypeName[9] =
		{
			"Default","Gray","RGB","RGBA","BGR",
			"BGRA","HSV","YUV","YCrCb",
		};

		bool isMaxContrast = false;
		bool isPseudoColor = false;
		bool isIgnoreAlpha = false;
		bool isContour = false;
	}_setting;

	QMenu _menu;
	QAction actionZoom_to_Fit;
	QAction actionZoom_to_Original_Size;
	QAction actionRotate_Clockwise_90;
	QAction actionRotate_Anticlockwise_90;
	QAction actionAuto_Maximize_Contrast;
	QAction actionPseudo_Color;
	QAction actionIgnore_Alpha;
	QAction actionDraw_Contour;
	

	QMenu _menu_Channel;
	QActionGroup actionGroup_Channel;
	QAction action_Channel_All;
	QAction action_Channel[4];

	QMenu _menu_Convert;
	QActionGroup actionGroup_Convert;
	QAction action_Convert[9];
	
	QAction actionDump_to_File;
	QAction actionLoad_From_File;
};
