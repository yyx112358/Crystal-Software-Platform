#include "stdafx.h"

#include <QDir>
#include <QFile>
#include <QtConcurrent>
#include <set>

#include "Brisque.h"

using namespace std;
using namespace cv; 
using namespace NewBrisque;
//using namespace OldBrisque;

int main(int argc, char *argv[])
{
#ifdef _DEBUG
	cv::RNG rng(411);
#else
	cv::RNG rng(static_cast<unsigned int>(time(nullptr)));
#endif // _DEBUG

	for (auto i = 0; i < argc; i++)
		cout << argv[i] << "   ";

	if (argc > 1)
	{
		switch (argv[1][0])
		{
		case '1'://计算BRISQUE特征向量
		{
			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures");
			auto filenames = dir.entryList(
				QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files, QDir::SortFlag::Name);
			size_t fileAmount = filenames.size();
			cv::Mat featureMat(fileAmount,NewBrisque::BRISQUE_FEATURE_LENGTH,
				NewBrisque::BRISQUE_MAT_TYPE,cv::Scalar(0));						
			int64 usedTime1 = 0, usedTime2 = 0, totalTime = getTickCount();

			#pragma omp parallel for//计算是相互独立的，可用并行处理加速
			for (int i = 0; i < fileAmount; i++)
			{
				string filename = dir.absoluteFilePath(filenames[i]).toStdString();
				Mat img = imread(filename, IMREAD_GRAYSCALE);				
				cout << filename << "\n";
				if (img.empty() == true)
					continue;

				cv::Mat feature1;				
				int64 timeBegin = cv::getTickCount();
				NewBrisque::ComputeBrisqueFeature(img, feature1);
				usedTime1 += cv::getTickCount() - timeBegin;
				featureMat.row(i) += feature1;

//				std::vector<double>feature2;
// 				timeBegin = cv::getTickCount();
// 				Old::ComputeBrisqueFeature(img, feature2);
// 				usedTime2 += cv::getTickCount() - timeBegin;
// 				//cv::Mat e = mscn1 - mscn2;
			}

			totalTime = cv::getTickCount() - totalTime;
			cout << "time1=" << usedTime1/getTickFrequency() << endl 
				<< "time2=" << usedTime2 / getTickFrequency() << endl
				<< "total=" << totalTime / getTickFrequency() << endl;
			cv::imwrite(dir.absoluteFilePath("feature.exr").toStdString(), featureMat);
		}
		break;
		case '2'://获得DMOS，写入一个txt文件
		{
			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures");
			//qDebug() << dir.entryList(QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files);
			QFile f(dir.absoluteFilePath("dmos.txt"));
			if (!f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text))
				break;

			QTextStream ts(&f);
			auto filenames = dir.entryList(
				QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files, QDir::SortFlag::Name);
			ts << "length" << ',' << filenames.size() << endl
				<< "dir" << ',' << dir.absolutePath() << endl;

			for (auto filename : filenames)
			{
				qDebug() << filename;
				ts << filename<<',';
				int n = static_cast<int>(rng.uniform(0, 8));
				for(auto i=0;i<n;i++)
					ts << ',' << rng.uniform(0.0, 1.0);
				ts << endl;
			}
		}
		break;
		case '3':
		{
			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures");
			cv::Mat featureMat, dmosMat;
			cv::Mat featureRange;
			std::set<int>invalidIdx;//无效样本（没有主观评分DMOS或featureMat无效（全0））
			
			//DMOS
			QFile fdmos(dir.absoluteFilePath("dmos.txt"));
			CV_Assert(fdmos.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
			QTextStream ts(&fdmos);
			ts.readLine();
			ts.readLine();
			for(int idx=0;ts.atEnd()==false;idx++)
			{
				std::vector<double>mos;
				bool b = false;
				double avgmos = 0;

				for (auto s : ts.readLine().split(','))
				{
					double d = s.toDouble(&b);
					if (b)	mos.push_back(d);
				}
				if (mos.size() > 0)
					avgmos = cv::mean(mos)[0];
				else
					invalidIdx.insert(idx);	

				dmosMat.push_back(avgmos);
			}
			fdmos.close();
			
			//特征矩阵
			featureMat = cv::imread(dir.absoluteFilePath("feature.exr").toStdString(), IMREAD_UNCHANGED);
			CV_Assert(featureMat.empty() == false && featureMat.cols == NewBrisque::BRISQUE_FEATURE_LENGTH);
			for (auto i = 0; i < featureMat.rows; i++)
				if (cv::countNonZero(featureMat.row(i) == 0))
					invalidIdx.insert(i);

			//去掉无效值
			cv::Mat trainData, trainLabel;
			for (auto i = 0; i < featureMat.rows; i++)
			{
				if (invalidIdx.count(i) == 0)
				{
					trainData.push_back(featureMat.row(i));
					trainLabel.push_back(dmosMat.row(i));
				}
			}

			for (auto i = 0; i < featureMat.cols; i++)
			{
				double minValue, maxValue;
				cv::minMaxLoc(featureMat.col(i), &minValue, &maxValue);
				cv::normalize(featureMat.col(i), featureMat.col(i), 1, 0, cv::NORM_MINMAX);
				featureRange.push_back(cv::Vec2d(minValue, maxValue));
			}
			auto psvm = cv::ml::SVM::create();
			if (trainData.type() == CV_64F)trainData.convertTo(trainData, CV_32F);
			if (trainLabel.type() == CV_64F)trainLabel.convertTo(trainLabel, CV_32F);
			auto dat = cv::ml::TrainData::create(trainData, cv::ml::ROW_SAMPLE, trainLabel);
			dat->setTrainTestSplitRatio(1 - 1.0 / 4, true);
			//psvm->setCoef0(0.0);//
			//psvm->setDegree(3);//
			psvm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 1e-3));
			//psvm->setGamma(0.5);
			psvm->setKernel(cv::ml::SVM::RBF);
			//psvm->setNu(0.5);//
			psvm->setP(0.5); // for EPSILON_SVR, epsilon in loss function?[未定]
							 //psvm->setC(1024); // From paper, soft classifier
			psvm->setType(cv::ml::SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
												 //	psvm->train(trainData, cv::ml::ROW_SAMPLE, trainLabel);
												 //psvm->trainAuto(trainData, cv::ml::ROW_SAMPLE, trainLabel);
												 //默认参数C=[0.1,500,5],GAMMA=[1e-5,0.6,15],P=[0.01,100,7]。参数满足minVal∗logStep^n<maxVal
			psvm->trainAuto(dat, 4, ml::ParamGrid(0.1, 100, 1.1), ml::ParamGrid(1e-5, 0.8, 2), ml::ParamGrid(0.01, 0.5, 2));//C,GAMMA,P【P必须<1否则找不到支持向量】
 			psvm->save(dir.absoluteFilePath("svm_model.txt").toStdString());
			cout << "<<<训练完成>>>" << endl
				<< "\tC=" << psvm->getC() << endl << "\tGamma=" << psvm->getGamma() << endl << "\tP=" << psvm->getP() << endl;
			cout << "error=" << psvm->calcError(dat, false, noArray()) << endl;
		}
		break;
		default:
			break;
		}
	}
	else
	{
		cout << "操作：\n"
			"1.计算BRISQUE特征向量\n"
			"2.获得DMOS\n"
			"3.训练BRISQUE\n"
			"4.预测BRISQUE\n"
			"~.退出\n" << endl;
	}
	system("pause");
	return 0;
}
