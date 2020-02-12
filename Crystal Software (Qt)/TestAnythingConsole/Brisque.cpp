#include "stdafx.h"
#include "Brisque.h"

#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>

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
		cv::FileStorage fs(settingPath, cv::FileStorage::WRITE);

		CV_Assert(lowerRange.size() > 0 && lowerRange.size() == upperRange.size());
		fs << "lower_range" << lowerRange;
		fs << "upper_range" << upperRange;

		CV_Assert(isTrained() == true);
		CV_Assert(psvm->getVarCount() == lowerRange.size());
		psvm->save(modelPath);
		fs << "model_path" << modelPath;
		fs << "is_trained" << isTrained();

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

		psvm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS,
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
		psvm->trainAuto(dat, 4, cv::ml::ParamGrid(0.1, 100, 1.1), cv::ml::ParamGrid(1e-5, 0.8, 2),
			cv::ml::ParamGrid(0.01, 0.5, 2));//C,GAMMA,P【P必须<1否则找不到支持向量】
		if (isDisplay)
		{
			std::cout << "<<<训练完成>>>" << endl
				<< "\tC=" << psvm->getC() << endl << "\tGamma=" << psvm->getGamma() << endl << "\tP=" << psvm->getP() << endl;
			std::cout << "error=" << psvm->calcError(dat, false, cv::noArray()) << endl;
		}

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
		std::set<int>invalidIdx;//无效样本（没有主观评分DMOS或featureMat无效（全0））

		//读取DMOS。如果某个项非空，则对其DMOS求均值，否则记为无效样本
		QFile f(QString::fromStdString(dmosPath));
		CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
		QTextStream ts(&f);
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
		featureMat = cv::imread(featurePath, cv::IMREAD_UNCHANGED);
		CV_Assert(featureMat.empty() == false && featureMat.cols == BRISQUE_FEATURE_LENGTH);
		for (auto i = 0; i < featureMat.rows; i++)//按行搜索，如果某一行全0则记为无效行
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
		return false;
	}
}

bool Brisque::Train(std::vector<std::string> featurePaths, std::string dmosPath)
{
	try
	{
		cv::Mat dmosMat;
		std::set<int>invalidIdx;//无效样本（没有主观评分DMOS或featureMat无效（全0））

		//读取DMOS。如果某个项非空，则对其DMOS求均值，否则记为无效样本
		QFile f(QString::fromStdString(dmosPath));
		CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
		QTextStream ts(&f);
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
		std::vector<cv::Mat>featureMats;
		for (auto featurePath : featurePaths)
			featureMats.push_back(cv::imread(featurePath, cv::IMREAD_UNCHANGED));
		
		const auto featureRows = dmosMat.rows;
		for (auto it = featureMats.begin(); it != featureMats.end();)//查找无效元素
		{
			if (it->empty() == true || it->rows != featureRows || it->cols != BRISQUE_FEATURE_LENGTH)
				it = featureMats.erase(it);
			else
			{
				for (auto i = 0; i < it->rows; i++)
					if (cv::countNonZero(it->row(i) == 0))
						invalidIdx.insert(i);
				++it;
			}
		}
		CV_Assert(featureMats.empty() == false);

		//去掉无效值
		cv::Mat trainData, trainLabel;
		std::cout << "无效行：";
		for (auto i = 0; i < featureRows; i++)
		{
			if (invalidIdx.count(i) == 0)
			{
				for(auto &featureMat:featureMats)
				{
					trainData.push_back(featureMat.row(i));
					trainLabel.push_back(dmosMat.row(i));
				}
			}
			else
				std::cout << i << ',';
		}

		return Train(trainData, trainLabel);
	}
	catch (cv::Exception&e)
	{
		return false;
	}
}

cv::Mat Brisque::Predict(cv::InputArray src) const
{
	cv::Mat result;
	try
	{
		CV_Assert(isTrained());
		std::vector<cv::Mat>imgs;
		switch (src.kind())
		{
		case cv::_InputArray::MAT:
		case cv::_InputArray::MATX:
		case cv::_InputArray::UMAT:
		case cv::_InputArray::EXPR:
			imgs.push_back(src.getMat());
			break;
		case cv::_InputArray::STD_VECTOR_MAT:
		case cv::_InputArray::STD_ARRAY_MAT:
			src.getMatVector(imgs);
			break;
		default:
			break;
		}
		for (auto img : imgs)
			result.push_back(psvm->predict(ComputeBrisqueFeature(img)));
	}
	catch (cv::Exception &e)
	{
	}
	return result;
}

cv::Mat Brisque::Predict(std::string filename) const
{
	cv::Mat result;
	try
	{
		std::string suffix = filename.substr(filename.find_last_of('.'), 1);

		if (suffix == "xml" || suffix == "yaml" || suffix == "json")//是CrystalSet形式
		{
			CV_Assert(!(suffix == "xml" || suffix == "yaml" || suffix == "json"));
		}
		else
			result = Predict(cv::imread(filename, cv::IMREAD_GRAYSCALE));
	}
	catch (cv::Exception& e)
	{
	}
	return result;
}

//计算MSCN系数
cv::Mat Brisque::CalcMSCN(cv::Mat inputImg)
{
	CV_Assert((inputImg.channels() == 1 || inputImg.channels() == 3));

	cv::Mat img;
	if (inputImg.channels() == 3)
		cv::cvtColor(inputImg, img, cv::COLOR_BGR2GRAY);
	else if (inputImg.channels() == 1)
		img = inputImg;
	else
		throw "Not support image without 1 or 3 channels";
	if (img.depth() != BRISQUE_MAT_TYPE)
		img.convertTo(img, BRISQUE_MAT_TYPE);

	// compute mu (local mean局部均值)
	cv::Mat mu;
	GaussianBlur(img, mu, cv::Size(7, 7), 7.0 / 6);

	//compute sigma (local sigma局部方差)
	cv::Mat sigma = img.mul(img);
	GaussianBlur(sigma, sigma, cv::Size(7, 7), 7.0 / 6);
	sigma -= mu.mul(mu);
	sigma = cv::abs(sigma);
	cv::sqrt(sigma, sigma);

	//计算MSCN系数
	cv::Mat structdis;/*(img - mu) / (sigma + 1)//拆开并使用原位运算，减少中间变量，从而提高速度*/
	structdis = img - mu;
	sigma += 1;//避免0除错误
	structdis /= sigma;
	return structdis;
}

void Brisque::AGGDfit(const cv::Mat&structdis, double& lsigma_best, double& rsigma_best, double& gamma_best)
{
	CV_Assert(structdis.depth() == BRISQUE_MAT_TYPE);

	long int poscount = 0, negcount = 0;
	double possqsum = 0, negsqsum = 0, abssum = 0;
	for (int i = 0; i < structdis.rows; i++)//发现这样写的速度超过了parallel_for和forEach，或许是因为不存在对共享变量的依赖同时内部存在着if？
	{
		BRISQUE_ELEMENT_TYPE*pRow = (BRISQUE_ELEMENT_TYPE*)(structdis.data + i*structdis.step);
		for (int j = 0; j < structdis.cols; j++)
		{
			double pt = pRow[j];
			if (pt > 0)
			{
				abssum += pt;
				possqsum += pt*pt;
				poscount++;
			}
			else if (pt < 0)
			{
				abssum -= pt;
				negsqsum += pt*pt;
				negcount++;
			}
		}
	}

	//	不能用forEach，因为涉及到对poscount等循环体外部变量的改写，存在多线程冲突使得结果错误
	// 	structdis.forEach<double>( [&poscount2,&negcount2,&possqsum2, &negsqsum2, &abssum2](double pixel,const int position[])->void
	// 	{
	// 		if (pixel > 0)
	// 		{
	// 			poscount2++;
	// 			possqsum2 += pixel*pixel;
	// 			abssum2 += pixel;
	// 		}
	// 		else if (pixel < 0)
	// 		{
	// 			negcount2++;
	// 			negsqsum2 += pixel*pixel;
	// 			abssum2 -= pixel;
	// 		}
	// 	});
	lsigma_best = cv::sqrt(negsqsum / negcount);
	rsigma_best = cv::sqrt(possqsum / poscount);

	double gammahat = lsigma_best / rsigma_best;
	long int totalcount = (structdis.cols)*(structdis.rows);
	double rhat = cv::pow(abssum / totalcount, 2.0) / ((negsqsum + possqsum) / totalcount);
	double rhatnorm = rhat*(cv::pow(gammahat, 3) + 1)*(gammahat + 1) / pow(pow(gammahat, 2) + 1, 2);

	double prevgamma = 0;
	double prevdiff = 1e10;
	float sampling = 0.001f;
	for (float gam = 0.2f; gam < 10; gam += sampling) //possible to coarsen sampling to quicken the code, with some loss of accuracy
	{
		double tg2 = tgamma(2 / gam);
		double r_gam = tg2*tg2 / (tgamma(1 / gam)*tgamma(3 / gam));
		double diff = abs(r_gam - rhatnorm);
		if (diff > prevdiff) break;
		prevdiff = diff;
		prevgamma = gam;
	}
	gamma_best = prevgamma;
}

cv::Mat Brisque::ComputeBrisqueFeature(const cv::Mat& orig)
{
	if (orig.rows <= 5 && orig.cols <= 5)
		return cv::Mat(BRISQUE_FEATURE_LENGTH, 1, BRISQUE_MAT_TYPE, cv::Scalar(0));
	cv::Mat orig_bw;
	// convert to grayscale 
	if (orig.channels() == 3)
		cvtColor(orig, orig_bw, cv::COLOR_BGR2GRAY);
	else
		orig_bw = orig.clone();
	// create a copy of original image	
	if (orig_bw.depth() != BRISQUE_MAT_TYPE)
		orig_bw.convertTo(orig_bw, BRISQUE_MAT_TYPE);

	// orig_bw now contains the grayscale image normalized to the range 0,1
	cv::Mat featureVector = cv::Mat(cv::Size(0, 0), BRISQUE_MAT_TYPE);
	int scalenum = 2; // number of times to scale the image
	for (int itr_scale = 1; itr_scale <= scalenum; itr_scale++)
	{
		// resize image
		cv::Size dst_size(orig_bw.cols / cv::pow((double)2, itr_scale - 1), orig_bw.rows / pow((double)2, itr_scale - 1));
		if (dst_size.height == 0)dst_size.height = 1;
		if (dst_size.width == 0)dst_size.width = 1;
		cv::Mat imdist_scaled;
		if (itr_scale == 1)
			imdist_scaled = orig_bw;
		else
			resize(orig_bw, imdist_scaled, dst_size, 0, 0, cv::INTER_NEAREST);

		// calculating MSCN coefficients
		cv::Mat structdis = CalcMSCN(imdist_scaled);

		// Compute AGGD fit to MSCN image
		double lsigma_best, rsigma_best, gamma_best;
		/*structdis = */AGGDfit(structdis, lsigma_best, rsigma_best, gamma_best);
		featureVector.push_back(gamma_best);
		featureVector.push_back((lsigma_best*lsigma_best + rsigma_best*rsigma_best) / 2);

		// Compute paired product images
		// indices for orientations (H, V, D1, D2)
		int shifts[4][2] = { { 0,1 },{ 1,0 },{ 1,1 },{ -1,1 } };
		cv::Mat shifted_structdis;
		for (int itr_shift = 0; itr_shift < 4; itr_shift++)
		{
			int shiftX = shifts[itr_shift][1], shiftY = shifts[itr_shift][0];
			cv::copyMakeBorder(structdis(cv::Rect(1, 1, structdis.cols - 2, structdis.rows - 2)), shifted_structdis,
				1 - shiftY, 1 + shiftY, 1 - shiftX, 1 + shiftX, cv::BORDER_CONSTANT, cv::Scalar::all(0));//向H,V方向偏移（因为copyMakeBorder不能扩展负数，所以写成这样）
			multiply(structdis, shifted_structdis, shifted_structdis);
			/*shifted_structdis = */AGGDfit(shifted_structdis, lsigma_best, rsigma_best, gamma_best);

			double constant = sqrt(tgamma(1 / gamma_best)) / sqrt(tgamma(3 / gamma_best));
			double meanparam = (rsigma_best - lsigma_best)*(tgamma(2 / gamma_best) / tgamma(1 / gamma_best))*constant;

			// push the calculated parameters from AGGD fit to pair-wise products
			featureVector.push_back(gamma_best);
			featureVector.push_back(meanparam);
			featureVector.push_back(cv::pow(lsigma_best, 2));
			featureVector.push_back(cv::pow(rsigma_best, 2));
		}

	}
	/*
	vector<double>test{ 0.851992,0.168142,0.433998,0.0291638,0.0297237,0.052905,0.439998,0.0305187,0.0280048,0.0515923,0.441998,-0.00765173,0.0426343,0.0366834,0.445998,-0.0114788,0.0435619,0.034746,0.903992,0.227309,0.430998,-0.0142514,0.0891312,0.073052,0.447998,-0.0438846,0.104758,0.0569214,0.444998,-0.025868,0.0885601,0.0611293,0.447998,-0.036822,0.094934,0.056039, };

	for (auto i=0;i<featurevector.size();i++)
	{
	//cout << featurevector[i] << ",";
	cout << featurevector[i] << "\t" << test[i] << endl;
	assert(abs(featurevector[i] - test[i]) < 0.0001);
	}
	//	system("pause");
	cout << endl;*/
	featureVector = featureVector.reshape(1, 1);//变换为行向量
	featureVector.convertTo(featureVector, BRISQUE_MAT_TYPE);
	CV_Assert(featureVector.total() == BRISQUE_FEATURE_LENGTH);
	return featureVector;
}

