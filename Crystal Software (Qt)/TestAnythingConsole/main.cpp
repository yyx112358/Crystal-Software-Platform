#include "stdafx.h"

#include <QDir>
#include <QFile>
#include <QtConcurrent>
#include <set>

#include "Brisque.h"
#include "Crystal.h"

using namespace std;

//遍历paths，展开文件名【不检查文件有效性，不递归搜索】
//如果是文件夹，则搜索其下的支持的图像文件("*.png","*.jpg","*.jpeg","*.bmp")并加入【不含子文件夹】
//txt文件，则按行展开,每一行被认定为一个文件的路径并加入
//其它类型则直接加入
QStringList ExpandFilenames(QStringList paths);


const char *CONST_CHAR_ModifyDmosPythonContent();
int main(int argc, char *argv[])
{
#ifdef _DEBUG
	cv::RNG rng(411);
#else
	cv::RNG rng(static_cast<unsigned int>(time(nullptr)));
#endif // _DEBUG

	for (auto i = 0; i < argc; i++)
		std::cout << argv[i] << endl;
	
	try
	{
		cv::CommandLineParser parser(argc, argv, 
			"{help h usage ?    |		|帮助。}"
			"{@mode             |predict|模式：1-训练，2-训练集生成，其它-预测}"
			"{@setting          |       |设置}"
			"{@input            |       |输入}"
			"{@output           |       |输出}"
			"{addition          |       |附加信息}"
			"{display           |0      |是否显示}");
		parser.about("Application name v1.0.0");
		if (parser.has("help") == true)
			parser.printMessage();

		QString mode = QString::fromStdString(parser.get<std::string>("@mode")),
			setting = QString::fromStdString(parser.get<std::string>("@setting")),
			input = QString::fromStdString(parser.get<std::string>("@input")),
			output = QString::fromStdString(parser.get<std::string>("@output"));
		int displayLevel = parser.get<int>("display");
		if (parser.check() == false)
		{
			parser.printErrors();
			return -1;
		}
		if (mode == "1" || mode == "train")
		{

// 			QFile fpath(input.split(',')[0]),finfo(input.split(',')[1]),fout(output);
// 			CV_Assert(fpath.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text) == true
// 				&& finfo.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text) == true
// 				&& fout.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text) == true);
// 			
// 			QTextStream tspath(&fpath), tsinfo(&finfo);
// 			QDataStream ()
			//train 1 "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset" ""
			CrystalSetManager manager;
			manager.Load(input.toStdString());
			std::vector<CommonCrystalSet>vccs;
			vccs = manager.Read(0);
		}
		else if (mode == "2" || mode == "generate")//训练集生成模式
		{
			//generate 1000 "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset" "d:\Users\yyx11\Desktop\Saved Pictures\DMOS_2020-02-10"
			bool generateModeAmountParseSuccess = false;
			size_t amount = setting.toInt(&generateModeAmountParseSuccess);
			CV_Assert(generateModeAmountParseSuccess == true);

			if (input.contains(".crystalset"))
			{
				CV_Assert(input.count(',') == 0);//TODO:暂不支持多个CrystalSet
				qDebug() << QStringLiteral("正在加载数据集：") << input;

				//加载crystalset
				CrystalSetManager manager;
				CV_Assert(manager.Load(input.toStdString()) == true);
				qDebug() << QStringLiteral("数据集索引完成，图像总数：") << manager.size();

				auto vccs = manager.Read(0);
				size_t crystalAmount = 0;
				std::vector<int>offsets;
				for (const auto&ccs : vccs)//生成查找表，以便找到第i个晶体位于哪个CommonCrystalSet
				{
					offsets.push_back(crystalAmount);
					crystalAmount += ccs.size();
				}
				offsets.push_back(crystalAmount);
				qDebug() << QStringLiteral("数据集加载完成，晶体总数：") << crystalAmount;

				//生成随机数
				std::vector<int>crystalIdxs(amount,0);
				rng.fill(crystalIdxs, cv::RNG::UNIFORM, cv::Scalar(0), cv::Scalar(crystalAmount));
				std::function<int(int, vector<int>&)>F_BinSearch = [](int target, vector<int>&vi)->int
				{
					for (auto i = 0; i < vi.size(); i++)
						if (vi[i] <= target&&vi[i + 1] > target)
							return i;
					return 0;
				};

				//搜索并生成
				QDir dir(output);
				if (dir.exists() == false)
					CV_Assert(dir.mkpath(dir.path()));
				QFile f(dir.filePath("dmos.txt"));//DMOS文件
				CV_Assert(f.open(QIODevice::WriteOnly | QIODevice::Text));
				QTextStream ts(&f);
				for (auto idx : crystalIdxs)
				{
					int ccsIdx = F_BinSearch(idx, offsets);
					//vccs[ccsIdx][idx - offsets[ccsIdx]];
					QString newname=QString::fromStdString(vccs[ccsIdx].path());
					newname = QString("%1.%2.%3")
						.arg(newname.section('\\',-1)).arg(idx - offsets[ccsIdx]).arg("png");
					newname = dir.filePath(newname);
					qDebug() << idx << newname;
					ts << newname << ",\n";
					//获取图像并适当扩展
					auto originImg = vccs[ccsIdx][idx - offsets[ccsIdx]].Image();
					auto ct = vccs[ccsIdx][idx - offsets[ccsIdx]].Contour();
					double ratio = cv::contourArea(ct) / cv::boundingRect(ct).area();
					int adjustValue;
					if (ratio > 0.75)
						adjustValue = static_cast<int>(originImg.rows*1.0 / 2);//边缘扩展
					else if (ratio > 0.5)
						adjustValue = static_cast<int>(originImg.rows*1.0 / 4);//边缘扩展
					else
						adjustValue = static_cast<int>(originImg.rows*1.0 / 8);//边缘扩展
					originImg = originImg.adjustROI(adjustValue, adjustValue, adjustValue, adjustValue);
					//写入数据
					cv::imwrite(newname.toStdString(), originImg);
					vccs[ccsIdx].ReleaseImg();
				}
				f.close();
				//写入dmos修改脚本
				f.setFileName(dir.filePath(QStringLiteral("修改dmos文件夹为当前.py")));
				f.open(QIODevice::WriteOnly | QIODevice::Text);
				f.write(CONST_CHAR_ModifyDmosPythonContent());
				f.close();
			}
			else
			{
				//=====添加待处理文件名=====
				QStringList blocks = input.split(',');
				if (input.isEmpty())//空则默认为当前目录
					blocks.append(".");
				QStringList filenames = ExpandFilenames(blocks);
				//TODO:检查有效性，展开CrystalSet

				std::vector<int>selectIdxs;
				rng.fill(selectIdxs, cv::RNG::UNIFORM, cv::Scalar(0), cv::Scalar(filenames.size()));
				CV_Assert(0);
			}
		}
		else if (mode == "3" || mode == "update")
		{
			//update csv "d:\Users\yyx11\Desktop\Saved Pictures\olddata\Crystals_Image_Path_1.csv","d:\Users\yyx11\Desktop\Saved Pictures\olddata\Crystals_Info_1.csv","d:\Users\yyx11\Desktop\Saved Pictures\fig_batch1" "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset"
			CrystalSetManager::UpdateCSV(input.split(',')[0].toStdString(),
				input.split(',')[1].toStdString(), output.toStdString(), 
				input.split(',')[2].toStdString());
		}
		else//预测模式
		{			
			//=====加载模型和命令行=====
			//predict "d:\Users\yyx11\Desktop\Saved Pictures\brisque.json" "d:\Users\yyx11\Desktop\Saved Pictures\Crystal_   0.png","d:\Users\yyx11\Desktop\Saved Pictures\Crystal_   1.png","d:\Users\yyx11\Desktop\Saved Pictures\FileNames.txt","d:\Users\yyx11\Desktop\Saved Pictures"  "d:\Users\yyx11\Desktop\Saved Pictures\result.txt"
			Brisque brisque;
			CV_Assert(brisque.Load(setting.toStdString()) == true);

			//=====添加待处理文件名=====
			QStringList blocks = input.split(',');
			if (input.isEmpty())//空则默认为当前目录
				blocks.append(".");
			QStringList filenames = ExpandFilenames(blocks);			

			//=====计算结果=====
			size_t fileAmount = filenames.size();
			QStringList results;//计算结果，图片则结果为浮点数
			for (auto i = 0; i < fileAmount; i++)results.append("");
			std::atomic_int64_t usedTime1 = 0, totalTime = cv::getTickCount();
#ifndef _DEBUG
			#pragma omp parallel for//计算是相互独立的，可用并行处理加速
#endif // _DEBUG
			for (int i = 0; i < fileAmount; i++)
			{
				auto filename = filenames[i];
				int64 timeBegin = cv::getTickCount();
				cv::Mat img = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
				if (img.empty() == false) 
				{
					auto result = brisque.Predict(img);
					results[i] = QString::number(result.at<BRISQUE_ELEMENT_TYPE>(0));
					usedTime1 += cv::getTickCount() - timeBegin;
					qDebug() << filename << "\t" << results[i] << "\t"
						<< (cv::getTickCount() - timeBegin) * 1000 / cv::getTickFrequency() << "ms" << endl;
				}
				else
					qDebug() << filename << "\t" << "[Error]" << endl;				
			}
			//TODO:=====计算CrystalSet形式=====
			for (auto i = 0; i < fileAmount; i++)
			{
				auto filename = filenames[i];
				if (results[i].isEmpty() == false 
					&& filename.section('.', -1, -1).toLower()=="crystalset")
				{
					qDebug() << filename << "\t" << "will support soon!" << endl;
				}
			}

			if (output.isEmpty() == false)
			{
				QFile f(output);
				CV_Assert(f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text));
				QTextStream ts(&f);
				for (auto i = 0; i < fileAmount; i++)
					ts << filenames[i] << ',' << results[i] << endl;
			}

			totalTime = cv::getTickCount() - totalTime;
			cout << "time1=" << usedTime1 / cv::getTickFrequency() << endl
				<< "total=" << totalTime / cv::getTickFrequency() << endl;
		}

		system("pause");
		return 0;
	}
	catch (cv::Exception &e)
	{
		system("pause");
		return -1;
	}

// 	if (argc > 1)
// 	{
// 		switch (argv[1][0])
// 		{
// 		case '1'://计算BRISQUE特征向量
// 		{
// 			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures");
// 			auto filenames = dir.entryList(
// 				QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files, QDir::SortFlag::Name);
// 			size_t fileAmount = filenames.size();
// 			cv::Mat featureMat(fileAmount,NewBrisque::BRISQUE_FEATURE_LENGTH,
// 				NewBrisque::BRISQUE_MAT_TYPE,cv::Scalar(0));						
// 			int64 usedTime1 = 0, usedTime2 = 0, totalTime = getTickCount();
// 
// 			#pragma omp parallel for//计算是相互独立的，可用并行处理加速
// 			for (int i = 0; i < fileAmount; i++)
// 			{
// 				string filename = dir.absoluteFilePath(filenames[i]).toStdString();
// 				Mat img = imread(filename, IMREAD_GRAYSCALE);				
// 				cout << filename << "\n";
// 				if (img.empty() == true)
// 					continue;
// 
// 				cv::Mat feature1;				
// 				int64 timeBegin = cv::getTickCount();
// 				feature1 = NewBrisque::ComputeBrisqueFeature(img);
// 				usedTime1 += cv::getTickCount() - timeBegin;
// 				featureMat.row(i) += feature1;
// 
// //				std::vector<double>feature2;
// // 				timeBegin = cv::getTickCount();
// // 				Old::ComputeBrisqueFeature(img, feature2);
// // 				usedTime2 += cv::getTickCount() - timeBegin;
// // 				//cv::Mat e = mscn1 - mscn2;
// 			}
// 
// 			totalTime = cv::getTickCount() - totalTime;
// 			cout << "time1=" << usedTime1/getTickFrequency() << endl 
// 				<< "time2=" << usedTime2 / getTickFrequency() << endl
// 				<< "total=" << totalTime / getTickFrequency() << endl;
// 			cv::imwrite(dir.absoluteFilePath("feature.exr").toStdString(), featureMat);
// 		}
// 		break;
// 		case '2'://获得DMOS，写入一个txt文件
// 		{
// 			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures");
// 			//qDebug() << dir.entryList(QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files);
// 			QFile f(dir.absoluteFilePath("dmos.txt"));
// 			if (!f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text))
// 				break;
// 
// 			QTextStream ts(&f);
// 			auto filenames = dir.entryList(
// 				QStringList{ "*.png","*.jpg","*.jpeg","*.bmp" }, QDir::Files, QDir::SortFlag::Name);
// 			ts << "length" << ',' << filenames.size() << endl
// 				<< "dir" << ',' << dir.absolutePath() << endl;
// 
// 			for (auto filename : filenames)
// 			{
// 				qDebug() << filename;
// 				ts << filename;
// 				int n = static_cast<int>(rng.uniform(0, 8));
// 				for(auto i=0;i<n;i++)
// 					ts << ',' << rng.uniform(0.0, 1.0);
// 				ts << endl;
// 			}
// 		}
// 		break;
// 		case '3':
// 		{
// 			QDir dir("d:\\Users\\yyx11\\Desktop\\Saved Pictures"); 
// 			cv::Mat featureMat, dmosMat;
// 			cv::Mat featureRange;
// 			std::set<int>invalidIdx;//无效样本（没有主观评分DMOS或featureMat无效（全0））
// 			
// 			//DMOS
// 			QFile f(dir.absoluteFilePath("dmos.txt"));
// 			CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
// 			QTextStream ts(&f);
// 			ts.readLine();
// 			ts.readLine();
// 			for(int idx=0;ts.atEnd()==false;idx++)
// 			{
// 				std::vector<double>mos;
// 				bool b = false;
// 				double avgmos = 0;
// 
// 				for (auto s : ts.readLine().split(','))
// 				{
// 					double d = s.toDouble(&b);
// 					if (b)	mos.push_back(d);
// 				}
// 				if (mos.size() > 0)
// 					avgmos = cv::mean(mos)[0];
// 				else
// 					invalidIdx.insert(idx);	
// 
// 				dmosMat.push_back(avgmos);
// 			}
// 			f.close();
// 			
// 			//特征矩阵
// 			featureMat = cv::imread(dir.absoluteFilePath("feature.exr").toStdString(), IMREAD_UNCHANGED);
// 			CV_Assert(featureMat.empty() == false && featureMat.cols == NewBrisque::BRISQUE_FEATURE_LENGTH);
// 			for (auto i = 0; i < featureMat.rows; i++)
// 				if (cv::countNonZero(featureMat.row(i) == 0))
// 					invalidIdx.insert(i);
// 
// 			//去掉无效值
// 			cv::Mat trainData, trainLabel;
// 			for (auto i = 0; i < featureMat.rows; i++)
// 			{
// 				if (invalidIdx.count(i) == 0)
// 				{
// 					trainData.push_back(featureMat.row(i));
// 					trainLabel.push_back(dmosMat.row(i));
// 				}
// 			}
// 
// 			for (auto i = 0; i < featureMat.cols; i++)
// 			{
// 				double minValue, maxValue;
// 				cv::minMaxLoc(featureMat.col(i), &minValue, &maxValue);
// 				cv::normalize(featureMat.col(i), featureMat.col(i), 1, 0, cv::NORM_MINMAX);
// 				featureRange.push_back(cv::Vec2d(minValue, maxValue));
// 			}
// 			f.setFileName(dir.absoluteFilePath("svm_range.txt"));
// 			CV_Assert(f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text));
// 			ts.setDevice(&f);
// 			ts << "lower_bound";
// 			for (auto i = 0; i < featureRange.rows; i++)ts << ',' << featureRange.at<Vec2d>(i)[0];
// 			ts << endl << "higher_bound";
// 			for (auto i = 0; i < featureRange.rows; i++)ts << ',' << featureRange.at<Vec2d>(i)[1];
// 			f.close();
// 
// 			auto psvm = cv::ml::SVM::create();
// 			if (trainData.type() == CV_64F)trainData.convertTo(trainData, CV_32F);
// 			if (trainLabel.type() == CV_64F)trainLabel.convertTo(trainLabel, CV_32F);
// 			auto dat = cv::ml::TrainData::create(trainData, cv::ml::ROW_SAMPLE, trainLabel);
// 			dat->setTrainTestSplitRatio(1 - 1.0 / 4, true);
// 			psvm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 1e-3));
// 			psvm->setKernel(cv::ml::SVM::RBF);
// 			psvm->setP(0.5); // for EPSILON_SVR, epsilon in loss function?[未定]
// 							 //psvm->setC(1024); // From paper, soft classifier
// 			psvm->setType(cv::ml::SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
// 			//默认参数C=[0.1,500,5],GAMMA=[1e-5,0.6,15],P=[0.01,100,7]。参数满足minVal∗logStep^n<maxVal
// 			psvm->trainAuto(dat, 4, ml::ParamGrid(0.1, 100, 1.1), ml::ParamGrid(1e-5, 0.8, 2),
// 				ml::ParamGrid(0.01, 0.5, 2));//C,GAMMA,P【P必须<1否则找不到支持向量】
//  			psvm->save(dir.absoluteFilePath("svm_model.txt").toStdString());
// 			std::cout << "<<<训练完成>>>" << endl
// 				<< "\tC=" << psvm->getC() << endl << "\tGamma=" << psvm->getGamma() << endl << "\tP=" << psvm->getP() << endl;
// 			std::cout << "error=" << psvm->calcError(dat, false, noArray()) << endl;
// 		}
// 		break;
// 		default:
// 		{
// // 			cv::FileStorage fs("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json",cv::FileStorage::WRITE);
// // 			fs << "model_path" << "d:/Users/yyx11/Desktop/Saved Pictures/brisque_model.yaml"
// // 				<< "is_trained" << true
// // 				<< "lower_range" << vector<double>{0.906992, 0.0528103, 0.292, -0.00799058, 0.00132038, 0.00490567, 0.313999, 0.0059081, 0.0017432, 0.00527856, 0.426998, -0.0147346, 0.0022087, 0.00239792, 0.430998, -0.0121986, 0.00247298, 0.00251512, 0.970991, 0.0634048, 0.311999, -0.0551393, 0.000795766, 0.00784639, 0.304999, -0.0270027, 0.000657921, 0.00775896, 0.393998, -0.0578719, 0.00150337, 0.00308404, 0.401998, -0.0682939, 0.00127113, 0.00317927}
// // 			<< "upper_range" << vector<double>{2.35801, 0.167282, 0.856992, 0.0726248, 0.0232258, 0.041512, 0.748994, 0.133226, 0.0135068, 0.0808311, 0.876992, 0.0504255, 0.0234961, 0.0341401, 0.873992, 0.0470702, 0.0223031, 0.0320025, 4.79683, 0.345746, 1.42401, 0.168002, 0.0921716, 0.134285, 1.04799, 0.324835, 0.0611702, 0.313557, 1.38501, 0.12093, 0.0909112, 0.110173, 1.302, 0.0636105, 0.0940173, 0.059682};
// // 			fs.release();
// 			Brisque brisque;
// 			int reTrainLevel = 0;
// 			if (brisque.Load("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json") == false)
// 			{
// 				std::cout << "模型载入失败，即将重新开始训练" << endl;
// 				if (brisque.Train("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\feature.exr",
// 					"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\dmos.txt") == false)
// 				{
// 					std::cout << "打开特征文件和主观评分文件失败，即将重新获取" << endl;
// 					//TODO:重新计算并获取
// 					cv::Mat featureMat, dmosMat;
// 					if (brisque.Train(featureMat, dmosMat) == false)
// 					{
// 						cout << "重新计算失败，即将退出" << endl;
// 						break;
// 					}
// 				}
// 				brisque.Save("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json");
// 			}
// 			brisque.Predict(cv::imread("d:\\Users\\yyx11\\Desktop\\Saved Pictures\\Crystal_   0.png"));
// 		}
// 		break;
// 		}
// 	}
// 	else
// 	{
// 		std::cout << "操作：\n"
// 			"1.计算BRISQUE特征向量\n"
// 			"2.获得DMOS\n"
// 			"3.训练BRISQUE\n"
// 			"4.预测BRISQUE\n"
// 			"~.退出\n" << endl;
// 	}

}
QStringList ExpandFilenames(QStringList paths)
{
	QStringList filenames;
	for (auto path : paths)
	{
		QFileInfo fileInfo(path);
		if (fileInfo.isDir())//目录，遍历给定目录下所有支持图片(jpg,png,bmp,jpeg)
		{
			QDir dir(fileInfo.filePath());
			auto result = dir.entryList(
				QStringList{ "*.png","*.jpg","*.jpeg","*.bmp", },
				QDir::Files, QDir::SortFlag::Name);
			qDebug() << QStringLiteral("文件夹:") << fileInfo.filePath() << endl
				<< QStringLiteral("总数:") << result.size();
			for (auto &r : result)
				r = dir.filePath(r);
			filenames.append(result);
		}
		else if (fileInfo.isFile() && fileInfo.suffix().toLower() == "txt")//txt，txt则按行遍历其中所有文件名
		{
			QFile f(path);
			if (!f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text))
				continue;
			QTextStream ts(&f);
			while (ts.atEnd() == false)
			{
				auto line = ts.readLine();
				if (line.isEmpty() == false)
					filenames.append(ts.readLine());
			}
		}
		else//其余情况直接加入filenames（引号会被命令行程序自动去掉）
			filenames.append(path);
	}
	return filenames;
}
class SavePredictResultAtClose
{
public:
	SavePredictResultAtClose(QString outPath, const QStringList&filenames, const QStringList&results)
		:outPath(outPath), filenames(filenames), results(results)
	{}
	~SavePredictResultAtClose()
	{
		if (outPath.isEmpty() == false)
		{
			QFile f(outPath);
			CV_Assert(f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text));
			QTextStream ts(&f);
			for (auto i = 0; i < filenames.size(); i++)
				ts << filenames[i] << ',' << results[i] << endl;
		}
	}
	QString outPath;
	const QStringList&filenames;
	const QStringList&results;
};
const char *CONST_CHAR_ModifyDmosPythonContent()
{
return
"import os\
\
dir = os.path.abspath(os.path.curdir)\
f = open(os.path.join(dir, 'dmos.txt'), 'r')\
content = []\
if f:\
    for line in f:\
        sep = '/'\
        if line.count('\\\\') > 0:\
            sep = '\\\\'\
        content.append(os.path.join(dir, line.split(sep)[-1]))\
    f.close()\
\
if len(content)>0:\
    f = open(os.path.join(dir, 'dmos.txt'), 'w')\
    f.writelines(content)\
    f.close()";
}