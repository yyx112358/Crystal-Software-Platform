﻿#include "stdafx.h"

#include <QDir>
#include <QFile>
#include <QtConcurrent>
#include <set>

#include "Brisque.h"

using namespace std;
using namespace cv; 
using namespace NewBrisque;
//using namespace OldBrisque;

//	无参考图像质量评估算法：Brisque
class Brisque
{
public:
	Brisque() {}

	int RunCommand(int argc, char*argv[]);

	bool Load(std::string settingPath);
	bool Save(std::string settingPath = ".\\brisque.json",
		std::string modelPath=".\\brisque_model.json")const;
	void Clear();
	
	//使用featureMat和dmosMat训练，该函数假设数据全部有效
	bool Train(const cv::Mat& featureMat, const cv::Mat& dmosMat);
	bool Train(std::string featurePath,std::string dmosPath);
	bool Train(std::string workPath, std::vector<std::string>filters = { "*.png","*.jpg","*.jpeg","*.bmp" });
	double Predict(cv::Mat img)const { return 0; }

	cv::Mat CalcFeature(std::vector<string> paths);

	bool isDisplay = true;
	bool isTrained()const { return psvm.empty() == false && psvm->isTrained() == true; }

	static double Alg_SROCC();
	static cv::Mat Alg_ExtractFeature(cv::Mat);

	cv::Ptr<cv::ml::SVM> psvm;
	std::vector<double> lowerRange, upperRange;
	std::string feature_path;
	std::string dmos_path;
private:
};

bool Brisque::Load(std::string settingPath)
{
	try
	{
		Clear();
		cv::FileStorage fs(settingPath, cv::FileStorage::FORMAT_AUTO | cv::FileStorage::READ);
		CV_Assert(fs.isOpened() == true 
			&& fs["model_path"].empty() == false && fs["is_trained"].empty() == false
			&& fs["lower_range"].empty() == false && fs["upper_range"].empty() == false);
		
		psvm = cv::ml::SVM::load(fs["model_path"].string());
		std::cout << fs["model_path"].string() << endl;

		bool is_trained = false;
		fs["is_trained"] >> is_trained;
		is_trained &= psvm->isTrained();
		CV_Assert(is_trained == true);

		fs["lower_range"] >> lowerRange;
		fs["upper_range"] >> upperRange;

		return true;
	}
	catch (cv::Exception &e)
	{
		Clear();
		return false;
	}
}

bool Brisque::Save(std::string settingPath /*= ".\\brisque.json"*/, std::string modelPath/*=".\\brisque_model.json"*/) const
{
	try
	{
		FileStorage fs(settingPath, cv::FileStorage::WRITE);

		CV_Assert(lowerRange.size() > 0 && lowerRange.size() == upperRange.size());
		fs << "lower_range" << lowerRange;
		fs << "upper_range" << upperRange;

		CV_Assert(isTrained() == true);
		CV_Assert(psvm->getVarCount() == lowerRange.size());		
		psvm->save(modelPath);
		fs << "model_path" << modelPath;
		fs << "is_trained" << isTrained();

		if (feature_path.empty() == false)
			fs << "feature_path" << feature_path;
		if (dmos_path.empty() == false)
			fs << "dmos_path" << dmos_path;

		return true;
	}
	catch (cv::Exception& e)
	{
		return false;
	}
}

void Brisque::Clear()
{
	if (psvm.empty() == false) 
		psvm->clear(); 
	lowerRange.clear();
	upperRange.clear();
}

bool Brisque::Train(const cv::Mat& featureMat, const cv::Mat& dmosMat)
{
	try
	{
		CV_Assert(featureMat.empty() == false
			&& featureMat.rows == dmosMat.rows);

		//归一化
		for (auto i = 0; i < featureMat.cols; i++)
		{
			double minValue, maxValue;
			cv::minMaxLoc(featureMat.col(i), &minValue, &maxValue);
			cv::normalize(featureMat.col(i), featureMat.col(i), 1, 0, cv::NORM_MINMAX);
			lowerRange.push_back(minValue);
			upperRange.push_back(maxValue);
		}

		//准备数据
		cv::Mat trainData, trainLabel;
		psvm = cv::ml::SVM::create();
		featureMat.convertTo(trainData, CV_32F);
		dmosMat.convertTo(trainLabel, CV_32F);
		auto dat = cv::ml::TrainData::create(trainData, cv::ml::ROW_SAMPLE, trainLabel);
		dat->setTrainTestSplitRatio(1 - 1.0 / 4, true);

		psvm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 
#ifdef _DEBUG
10
#else
			1000
#endif // _DEBUG
, 1e-3));
		psvm->setKernel(cv::ml::SVM::RBF);
		psvm->setP(0.5); // for EPSILON_SVR, epsilon in loss function?[未定]
						 //psvm->setC(1024); // From paper, soft classifier
		psvm->setType(cv::ml::SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
											 
		//默认参数C=[0.1,500,5],GAMMA=[1e-5,0.6,15],P=[0.01,100,7]。参数满足minVal∗logStep^n<maxVal
		//训练
		psvm->trainAuto(dat, 4, ml::ParamGrid(0.1, 100, 1.1), ml::ParamGrid(1e-5, 0.8, 2),
			ml::ParamGrid(0.01, 0.5, 2));//C,GAMMA,P【P必须<1否则找不到支持向量】
		std::cout << "<<<训练完成>>>" << endl
			<< "\tC=" << psvm->getC() << endl << "\tGamma=" << psvm->getGamma() << endl << "\tP=" << psvm->getP() << endl;
		std::cout << "error=" << psvm->calcError(dat, false, noArray()) << endl;

		return true;
	}
	catch (cv::Exception&e)
	{
		Clear();
		return false;
	}
}

bool Brisque::Train(std::string featurePath, std::string dmosPath)
{
	try
	{
		cv::Mat featureMat, dmosMat;
		cv::Mat featureRange;
		std::set<int>invalidIdx;//无效样本（没有主观评分DMOS或featureMat无效（全0））

		//DMOS
		QFile f(QString::fromStdString(dmosPath));
		CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
		QTextStream ts(&f);
		ts.readLine();
		ts.readLine();
		for (int idx = 0; ts.atEnd() == false; idx++)
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
		f.close();

		//特征矩阵
		featureMat = cv::imread(featurePath, IMREAD_UNCHANGED);
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

		return Train(trainData, trainLabel);
	}
	catch (cv::Exception&e)
	{
		Clear();
		return false;
	}
}

int main(int argc, char *argv[])
{
#ifdef _DEBUG
	cv::RNG rng(411);
#else
	cv::RNG rng(static_cast<unsigned int>(time(nullptr)));
#endif // _DEBUG

	for (auto i = 0; i < argc; i++)
		std::cout << argv[i] << "   ";
	//mode:计算feature、获得dmos、训练、预测
	//isDisplay
	//RetryLevel	重试级别，默认4：//0：无需训练;1:使用已有feature和dmos;2.重新获得feature，使用已有dmos
			//3.使用已有的feature，重新获得dmos；4.重新获得feature和dmos
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
				ts << filename;
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
			QFile f(dir.absoluteFilePath("dmos.txt"));
			CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
			QTextStream ts(&f);
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
			f.close();
			
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
			f.setFileName(dir.absoluteFilePath("svm_range.txt"));
			CV_Assert(f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text));
			ts.setDevice(&f);
			ts << "lower_bound";
			for (auto i = 0; i < featureRange.rows; i++)ts << ',' << featureRange.at<Vec2d>(i)[0];
			ts << endl << "higher_bound";
			for (auto i = 0; i < featureRange.rows; i++)ts << ',' << featureRange.at<Vec2d>(i)[1];
			f.close();

			auto psvm = cv::ml::SVM::create();
			if (trainData.type() == CV_64F)trainData.convertTo(trainData, CV_32F);
			if (trainLabel.type() == CV_64F)trainLabel.convertTo(trainLabel, CV_32F);
			auto dat = cv::ml::TrainData::create(trainData, cv::ml::ROW_SAMPLE, trainLabel);
			dat->setTrainTestSplitRatio(1 - 1.0 / 4, true);
			psvm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 1e-3));
			psvm->setKernel(cv::ml::SVM::RBF);
			psvm->setP(0.5); // for EPSILON_SVR, epsilon in loss function?[未定]
							 //psvm->setC(1024); // From paper, soft classifier
			psvm->setType(cv::ml::SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
			//默认参数C=[0.1,500,5],GAMMA=[1e-5,0.6,15],P=[0.01,100,7]。参数满足minVal∗logStep^n<maxVal
			psvm->trainAuto(dat, 4, ml::ParamGrid(0.1, 100, 1.1), ml::ParamGrid(1e-5, 0.8, 2),
				ml::ParamGrid(0.01, 0.5, 2));//C,GAMMA,P【P必须<1否则找不到支持向量】
 			psvm->save(dir.absoluteFilePath("svm_model.txt").toStdString());
			std::cout << "<<<训练完成>>>" << endl
				<< "\tC=" << psvm->getC() << endl << "\tGamma=" << psvm->getGamma() << endl << "\tP=" << psvm->getP() << endl;
			std::cout << "error=" << psvm->calcError(dat, false, noArray()) << endl;
		}
		break;
		default:
		{
// 			cv::FileStorage fs("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json",cv::FileStorage::WRITE);
// 			fs << "model_path" << "d:/Users/yyx11/Desktop/Saved Pictures/brisque_model.yaml"
// 				<< "is_trained" << true
// 				<< "lower_range" << vector<double>{0.906992, 0.0528103, 0.292, -0.00799058, 0.00132038, 0.00490567, 0.313999, 0.0059081, 0.0017432, 0.00527856, 0.426998, -0.0147346, 0.0022087, 0.00239792, 0.430998, -0.0121986, 0.00247298, 0.00251512, 0.970991, 0.0634048, 0.311999, -0.0551393, 0.000795766, 0.00784639, 0.304999, -0.0270027, 0.000657921, 0.00775896, 0.393998, -0.0578719, 0.00150337, 0.00308404, 0.401998, -0.0682939, 0.00127113, 0.00317927}
// 			<< "upper_range" << vector<double>{2.35801, 0.167282, 0.856992, 0.0726248, 0.0232258, 0.041512, 0.748994, 0.133226, 0.0135068, 0.0808311, 0.876992, 0.0504255, 0.0234961, 0.0341401, 0.873992, 0.0470702, 0.0223031, 0.0320025, 4.79683, 0.345746, 1.42401, 0.168002, 0.0921716, 0.134285, 1.04799, 0.324835, 0.0611702, 0.313557, 1.38501, 0.12093, 0.0909112, 0.110173, 1.302, 0.0636105, 0.0940173, 0.059682};
// 			fs.release();
			Brisque brisque;
			int reTrainLevel = 0;
			if (brisque.Load("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json") == false)
			{
				std::cout << "模型载入失败，即将重新开始训练" << endl;
				if (brisque.Train("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\feature.exr",
					"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\dmos.txt") == false)
				{
					std::cout << "打开特征文件和主观评分文件失败，即将重新获取" << endl;
					//TODO:重新计算并获取
					cv::Mat featureMat, dmosMat;
					if (brisque.Train(featureMat, dmosMat) == false)
					{
						cout << "重新计算失败，即将退出" << endl;
						break;
					}
				}
				brisque.Save("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json");
			}
			brisque.Predict(cv::imread("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\Crystal_   0.png"));
		}
		break;
		}
	}
	else
	{
		std::cout << "操作：\n"
			"1.计算BRISQUE特征向量\n"
			"2.获得DMOS\n"
			"3.训练BRISQUE\n"
			"4.预测BRISQUE\n"
			"~.退出\n" << endl;
	}
	system("pause");
	return 0;
}

