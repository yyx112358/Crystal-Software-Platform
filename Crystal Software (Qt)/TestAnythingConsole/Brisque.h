#pragma once
//Reference:https://www.learnopencv.com/image-quality-assessment-brisque/
namespace NewBrisque
{
#define BRISQUE_SINGLE_PRECISION//以单精度运算BRISQUE，以一定的精度损失换来一倍的效率提升
#ifndef BRISQUE_SINGLE_PRECISION
	const int BRISQUE_MAT_TYPE = CV_64F;
	typedef double BRISQUE_ELEMENT_TYPE;
#else
	const int BRISQUE_MAT_TYPE = CV_32F;
	typedef float BRISQUE_ELEMENT_TYPE;
#endif
	const size_t BRISQUE_FEATURE_LENGTH = 36;//特征向量长度
	//计算MSCN系数
	cv::Mat CalcMSCN(cv::Mat inputImg)
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

	// function to compute best fit parameters from AGGDfit 
	void AGGDfit(const cv::Mat&structdis, double& lsigma_best, double& rsigma_best, double& gamma_best)
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

	//计算BRISQUE特征向量（输出BRISQUE_MAT_TYPE类型行向量）
	cv::Mat ComputeBrisqueFeature(const cv::Mat& orig)
	{
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
		assert(featureVector.total() == BRISQUE_FEATURE_LENGTH);
		return featureVector;
	}

	//void NormalizeBrisqueFeatureArray()
}
/*
namespace OldBrisque
{
	template<class T> class Image
	{
	private:

		cv::Mat imgp;
	public:
		Image(cv::Mat img = 0)
		{
			imgp = img.clone();
		}
		~Image()
		{
			imgp = 0;
		}
		cv::Mat equate(cv::Mat img)
		{
			img = imgp.clone();
			return img;
		}
		void showimage() {
			cv::imshow("imgp", imgp);
			cv::waitKey(0);
			cv::destroyAllWindows();
		}
		inline T* operator[](const int rowIndx)
		{
			//imgp->data and imgp->width
			return (T*)(imgp.data + rowIndx*imgp.step);
		}
	};

	typedef Image<double> BwImage;
	cv::Mat AGGDfit(cv::Mat structdis, double& lsigma_best, double& rsigma_best, double& gamma_best);

	void ComputeBrisqueFeature(const cv::Mat& orig, std::vector<double>& featurevector)
	{
		cv::Mat orig_bw_int;
		// convert to grayscale 
		if (orig.channels() == 3)
			cvtColor(orig, orig_bw_int, cv::COLOR_BGR2GRAY);
		else
			orig_bw_int = orig.clone();
		// create a copy of original image
		cv::Mat orig_bw(orig_bw_int.size(), CV_64FC1, 1);
		orig_bw_int.convertTo(orig_bw, 1.0 / 255);
		orig_bw_int.release();

		// orig_bw now contains the grayscale image normalized to the range 0,1

		int scalenum = 2; // number of times to scale the image
		for (int itr_scale = 1; itr_scale <= scalenum; itr_scale++)
		{
			// resize image
			cv::Size dst_size(orig_bw.cols / cv::pow((double)2, itr_scale - 1), orig_bw.rows / pow((double)2, itr_scale - 1));
			if (dst_size.height == 0)dst_size.height = 1;
			if (dst_size.width == 0)dst_size.width = 1;
			cv::Mat imdist_scaled;
			resize(orig_bw, imdist_scaled, dst_size, 0, 0, cv::INTER_NEAREST);
			imdist_scaled.convertTo(imdist_scaled, CV_64FC1, 1.0 / 255.0);
			// calculating MSCN coefficients
			// compute mu (local mean)
			cv::Mat mu(imdist_scaled.size(), CV_64FC1, 1);
			GaussianBlur(imdist_scaled, mu, cv::Size(7, 7), 1.166);

			cv::Mat mu_sq;
			cv::pow(mu, double(2.0), mu_sq);

			//compute sigma (local sigma)
			cv::Mat sigma(imdist_scaled.size(), CV_64FC1, 1);
			cv::multiply(imdist_scaled, imdist_scaled, sigma);
			GaussianBlur(sigma, sigma, cv::Size(7, 7), 1.166);

			cv::subtract(sigma, mu_sq, sigma);
			cv::pow(sigma, double(0.5), sigma);
			add(sigma, cv::Scalar(1.0 / 255), sigma); // to avoid DivideByZero Error

			cv::Mat structdis(imdist_scaled.size(), CV_64FC1, 1);
			subtract(imdist_scaled, mu, structdis);
			divide(structdis, sigma, structdis);  // =======structdis is [MSCN image]========

												  // Compute AGGD fit to MSCN image
			double lsigma_best, rsigma_best, gamma_best;

			structdis = AGGDfit(structdis, lsigma_best, rsigma_best, gamma_best);
			featurevector.push_back(gamma_best);
			featurevector.push_back((lsigma_best*lsigma_best + rsigma_best*rsigma_best) / 2);

			// Compute paired product images
			// indices for orientations (H, V, D1, D2)
			int shifts[4][2] = { { 0,1 },{ 1,0 },{ 1,1 },{ -1,1 } };
			for (int itr_shift = 1; itr_shift <= 4; itr_shift++)
			{
				// select the shifting index from the 2D array
				int* reqshift = shifts[itr_shift - 1];

				// declare shifted_structdis as pairwise image
				cv::Mat shifted_structdis(imdist_scaled.size(), CV_64F, 1);

				// create copies of the images using BwImage constructor
				// utility constructor for better subscript access (for pixels)
				BwImage OrigArr(structdis);
				BwImage ShiftArr(shifted_structdis);

				// create pair-wise product for the given orientation (reqshift)
				//TODO: 之后可以用foreach的lambda表达式
				for (int i = 0; i < structdis.rows; i++)
				{
					for (int j = 0; j < structdis.cols; j++)
					{
						if (i + reqshift[0] >= 0 && i + reqshift[0] < structdis.rows && j + reqshift[1] >= 0 && j + reqshift[1] < structdis.cols)
						{
							ShiftArr[i][j] = OrigArr[i + reqshift[0]][j + reqshift[1]];
						}
						else
						{
							ShiftArr[i][j] = 0;
						}
					}
				}

				// cv::Mat structdis_pairwise;
				shifted_structdis = ShiftArr.equate(shifted_structdis);

				// calculate the products of the pairs
				multiply(structdis, shifted_structdis, shifted_structdis);

				// fit the pairwise product to AGGD 
				shifted_structdis = AGGDfit(shifted_structdis, lsigma_best, rsigma_best, gamma_best);

				double constant = sqrt(tgamma(1 / gamma_best)) / sqrt(tgamma(3 / gamma_best));
				double meanparam = (rsigma_best - lsigma_best)*(tgamma(2 / gamma_best) / tgamma(1 / gamma_best))*constant;

				// push the calculated parameters from AGGD fit to pair-wise products
				featurevector.push_back(gamma_best);
				featurevector.push_back(meanparam);
				featurevector.push_back(cv::pow(lsigma_best, 2));
				featurevector.push_back(cv::pow(rsigma_best, 2));
			}
		}
	}

	// function to compute best fit parameters from AGGDfit 
	cv::Mat AGGDfit(cv::Mat structdis, double& lsigma_best, double& rsigma_best, double& gamma_best)
	{
		// create a copy of an image using BwImage constructor (brisque.h - more info)
		BwImage ImArr(structdis);

		long int poscount = 0, negcount = 0;
		double possqsum = 0, negsqsum = 0, abssum = 0;
		for (int i = 0; i < structdis.rows; i++)
		{
			for (int j = 0; j < structdis.cols; j++)
			{
				double pt = ImArr[i][j]; // BwImage provides [][] access
				if (pt > 0)
				{
					poscount++;
					possqsum += pt*pt;
					abssum += pt;
				}
				else if (pt < 0)
				{
					negcount++;
					negsqsum += pt*pt;
					abssum -= pt;
				}
			}
		}

		lsigma_best = cv::pow(negsqsum / negcount, 0.5);
		rsigma_best = cv::pow(possqsum / poscount, 0.5);

		double gammahat = lsigma_best / rsigma_best;
		long int totalcount = (structdis.cols)*(structdis.rows);
		double rhat = cv::pow(abssum / totalcount, static_cast<double>(2)) / ((negsqsum + possqsum) / totalcount);
		double rhatnorm = rhat*(cv::pow(gammahat, 3) + 1)*(gammahat + 1) / pow(pow(gammahat, 2) + 1, 2);

		double prevgamma = 0;
		double prevdiff = 1e10;
		float sampling = 0.001f;
		for (float gam = 0.2f; gam < 10; gam += sampling) //possible to coarsen sampling to quicken the code, with some loss of accuracy
		{
			double r_gam = tgamma(2 / gam)*tgamma(2 / gam) / (tgamma(1 / gam)*tgamma(3 / gam));
			double diff = abs(r_gam - rhatnorm);
			if (diff > prevdiff) break;
			prevdiff = diff;
			prevgamma = gam;
		}
		gamma_best = prevgamma;

		return structdis.clone();
	}
}*/