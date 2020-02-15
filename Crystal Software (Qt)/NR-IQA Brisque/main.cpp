#include "stdafx.h"

#include <QtCore\QDir>
#include <QtCore\QFile>
#include <QtConcurrent\QtConcurrent>

#include "Brisque.h"
#include "Crystal.h"

using namespace std;

//遍历paths，展开文件名【不检查文件有效性，不递归搜索】
//如果是文件夹，则搜索其下的支持的图像文件("*.png","*.jpg","*.jpeg","*.bmp")并加入【不含子文件夹】
//txt文件，则按行展开,每一行被认定为一个文件的路径并加入
//其它类型则直接加入
QStringList ExpandFilenames(QStringList paths);

void Predict_CrystalSet_Reduce(QStringList &result, const QString &s)
{
	result.append(s);
}
const char *CONST_CHAR_ModifyDmosPythonContent();
const char *CONST_CHAR_CommanLineParserAboutMessage();
int main(int argc, char *argv[])
{
#ifdef _DEBUG
	cv::RNG rng(411);
	srand(411);
#else
	cv::RNG rng(static_cast<unsigned int>(time(nullptr)));
	srand(time(nullptr));
#endif // _DEBUG

#ifdef _DEBUG
	for (auto i = 0; i < argc; i++)
		std::cout << argv[i] << endl;
#endif // _DEBUG
	
#ifndef _DEBUG
	try
#endif // _DEBUG
	{
		cv::CommandLineParser parser(argc, argv, 
			"{help h usage ?      |		  |帮助。}"
			"{@mode               |predict|模式。train-训练，generate-训练集生成，update-数据集升级，其它-预测}"
			"{@setting            |       |设置。依模式不同而不同}"
			"{@input              |       |输入。该参数当中不能出现空格，如果出现空格必须用双引号括起}"
			"{@output             |       |输出。该参数当中不能出现空格，如果出现空格必须用双引号括起}"
			"{addition            |       |附加信息}"
			"{display             |0      |是否显示运行信息}"
			"{train_expand_sample |1      |是否扩展样本。\n"
			"                取值>1意味着扩展样本，将强制重新生成特征矩阵，目前最多扩展8倍。\n"
			"                扩展方式包括：是/否垂直翻转+旋转90/180/270/0°}"
			"{generate_reset      |       |是否清空样本。\n"
			"                给出，则清空输出文件夹下dmos.txt和训练集图像\n"
			"                不给出，则不清空输出文件夹下训练集图像，将建立名称不同的dmos文件}");
		parser.about(CONST_CHAR_CommanLineParserAboutMessage());
		if (parser.has("h") == true)
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
			//输入参数检查
			//train 2 -train_expand_sample=8 "d:\Users\yyx11\Desktop\Saved Pictures\BRISQUE_2020-02-10\dmos.txt" "d:\Users\yyx11\Desktop\Saved Pictures\BRISQUE_2020-02-10"
			int trainLevel = setting.toInt();//训练等级。请参考CONST_CHAR_CommanLineParserAboutMessage()中信息
			int expandLevel;//扩展等级，代表扩展出n个图像
			if (parser.has("train_expand_sample")) 
			{
				expandLevel = parser.get<int>("train_expand_sample");
				if (expandLevel < 1)
					expandLevel = 1; 
				else if (expandLevel > 8)
					expandLevel = 8;
			}
			else
				expandLevel = 1;
			CV_Assert(QFileInfo(output).isDir() == true);
			auto filename_dmos = input.section(',', 0, 1);//第0个为dmos文件
			CV_Assert(QFileInfo(filename_dmos).isFile());
			if (QFileInfo(filename_dmos).suffix().toLower() != "txt")
				qWarning().noquote() << QString("DMOS file [%1] is not a txt file. May not support")
				.arg(filename_dmos);
			auto filename_features = input.split(',');//其余为feature文件
			filename_features.pop_front();
			
			if (trainLevel <= 0)//0级，什么都不做
				return 0;

			if (trainLevel == 3 || trainLevel == 4)//重新获得dmos
				throw cv::Exception(0, "请使用python工具重新获得DMOS\n"
					"如果需要重新获得dmos.txt，请使用generate模式",
					__FUNCTION__, __FILE__, __LINE__);
			
			if (trainLevel == 2 || trainLevel == 4 )//重新获得feature
			{
				qDebug() << QStringLiteral("即将重新获得特征矩阵，个数：") << expandLevel;
				QFile f(filename_dmos);
				CV_Assert(f.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text));
				QTextStream ts(&f);
				QStringList filenames;
				for (int idx = 0; ts.atEnd() == false; idx++)
				{
					auto line = ts.readLine();
					if (line.isEmpty() == false)
						filenames.append(line.split(',')[0]);
				}
				f.close();

				std::vector<cv::Mat>featureMats(expandLevel,
					cv::Mat(filenames.size(), BRISQUE_FEATURE_LENGTH, CV_32F,cv::Scalar(0)));
				size_t fileAmount = filenames.size();
				int64 totalTime = cv::getTickCount();
			#pragma omp parallel for//计算是相互独立的，可用并行处理加速
				for (int i = 0; i < fileAmount; i++)
				{
					auto filename = filenames[i].toStdString();
					cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
					cout << filename << "\n";
					if (img.rows <= 5 || img.cols <= 5)//不可过小
						continue;
					for (auto j = 0; j < featureMats.size(); j++)
					{
						cv::Mat feature1 = img;
						//扩展，对第j张，如果j是奇数，则垂直翻转;如果j>>1为1、2、3，则旋转90/180/270
						if (j & 0x01 == 1)cv::flip(feature1, feature1, 0);
						if (j >> 1 == 1)cv::rotate(feature1, feature1, cv::ROTATE_90_CLOCKWISE);
						else if (j >> 1 == 2)cv::rotate(feature1, feature1, cv::ROTATE_180);
						else if (j >> 1 == 3)cv::rotate(feature1, feature1, cv::ROTATE_90_COUNTERCLOCKWISE);
						feature1 = Brisque::ComputeBrisqueFeature(feature1);
						featureMats[j].row(i) += feature1;
					}
				}
				totalTime = cv::getTickCount() - totalTime;
				cout << "total=" << totalTime / cv::getTickFrequency() << endl;

				//重新写入features
				filename_features.clear();
				for (auto i = 0; i < featureMats.size(); i++)
				{
					QString featureName = filename_dmos;
					featureName.chop(featureName.section('.', -1).size()+1);
					featureName = featureName + "_" + QString::number(i) + ".exr";
					cv::imwrite(featureName.toStdString(), featureMats[i]);
					filename_features.append(featureName);
				}
			}

			Brisque brisque;
			std::vector<std::string>std_filename_features;
			for (auto s : filename_features)
				std_filename_features.push_back(s.toStdString());
			qDebug() << "即将开始训练";
			CV_Assert(brisque.Train(std_filename_features, filename_dmos.toStdString()));
			qDebug() << "训练完成，将保存文件到：" << output + "\\brisque.json" << output + "\\brisque_model.json";
			CV_Assert(brisque.Save((output + "\\brisque.json").toStdString(), 
				(output + "\\brisque_model.json").toStdString()));
		}
		else if (mode == "2" || mode == "generate")//训练集生成模式
		{
			//generate 1000 "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset" "d:\Users\yyx11\Desktop\Saved Pictures\BRISQUE_2020-02-10"
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

				auto vccs = manager.Read(0);//晶体集合
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
				std::function<int(int, vector<int>&)>F_BinSearch 
					= [](int target, vector<int>&data_list)->int//二分查找，搜索target晶体对应的图片位置
				{
					int low = 0, high = data_list.size() - 1;
					while (low <= high)
					{
						int mid = (low + high) / 2;
						if (data_list[mid] == target)
						{
							while (mid + 1 < data_list.size() && data_list[mid + 1] == target)
								mid++;
							return mid;
						}
						else if (data_list[mid] > target)
							high = mid - 1;
						else
							low = mid + 1;
					}
					return high;
// 					for (auto i = 0; i < vi.size(); i++)
// 						if (vi[i] <= target&&vi[i + 1] > target)
// 							return i;
// 					return 0;
				};

				//搜索并生成
				QDir dir(output);
				QFile f(dir.filePath("dmos.txt"));//DMOS文件
				if (dir.exists() == false)
					CV_Assert(dir.mkpath(dir.path()));
				if (parser.has("generate_reset") == true)
				{
					qWarning() << QStringLiteral("-generate_reset选项被选中，将删除旧的数据");
					for (auto filename : dir.entryList(
						QStringList{ "*.png","*.jpg","*.jpeg","*.bmp", },
						QDir::Files, QDir::SortFlag::Name))
						qWarning() << "Removing" << filename << dir.remove(filename);
				}
				if (parser.has("generate_reset") == false && dir.exists("dmos.txt") == true)
				{
					//如果不重置，则重命名
					f.setFileName(dir.filePath(QString("dmos_%1.txt")
						.arg(QDateTime::currentDateTime().toSecsSinceEpoch())));
					CV_Assert(f.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Text));
				}
				else
					CV_Assert(f.open( QIODevice::WriteOnly | QIODevice::Text));

				qInfo() << QStringLiteral("开始生成训练集，总数：") << amount;
				QTextStream ts(&f);
				//#pragma omp parallel for
				for (auto idx : crystalIdxs)
				{
					int ccsIdx = F_BinSearch(idx, offsets);

					#ifdef _DEBUG
					int n = static_cast<int>(rng.uniform(0, 8));
					for(auto i=0;i<n;i++)
						ts << ',' << rng.uniform(0.0, 1.0);
					ts << "\n";
					#else
					ts << ",\n";
					#endif // _DEBUG
					//获取图像并适当扩展
					auto originImg = vccs[ccsIdx][idx - offsets[ccsIdx]].Image();
					if (originImg.rows < 10 || originImg.cols < 10)
						continue;
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
					
					//重命名名称。格式：原文件名.晶体在CommonCrystalSet中序号.png
					QString newname = QString::fromStdString(vccs[ccsIdx].path());
					newname = QString("%1.%2.%3")
						.arg(newname.section('\\', -1)).arg(idx - offsets[ccsIdx]).arg("png");
					newname = dir.filePath(newname);
					qDebug() << idx << newname;
					ts << newname;
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
			if (setting.toLower() == "csv")
			{
				//update csv "d:\Users\yyx11\Desktop\Saved Pictures\olddata\Crystals_Image_Path_1.csv","d:\Users\yyx11\Desktop\Saved Pictures\olddata\Crystals_Info_1.csv","d:\Users\yyx11\Desktop\Saved Pictures\fig_batch1" "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset"
				CrystalSetManager::UpdateCSV(input.split(',')[0].toStdString(),
					input.split(',')[1].toStdString(), output.toStdString(),
					input.split(',')[2].toStdString());
			}
			else if (setting.toLower() == "update_dir")
			{
				//update update_dir "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset","d:\Users\yyx11\Desktop\Saved Pictures\fig_batch1" "d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset"
				CrystalSetManager::UpdateDir(input.split(',')[0].toStdString(),
					input.split(',')[1].toStdString());
			}
			else
				std::cout << "setting [" << setting.toStdString() << "] is not support" << endl;
		}
		else//预测模式
		{			
			//=====加载模型和命令行=====
			//predict "d:\Users\yyx11\Desktop\Saved Pictures\brisque.json" "d:\Users\yyx11\Pictures\lovewallpaper\1.jpg","d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset","d:\Users\yyx11\Desktop\Saved Pictures\FileNames.txt","d:\Users\yyx11\Desktop\Saved Pictures"  "d:\Users\yyx11\Desktop\Saved Pictures\result.txt"
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
					results[i] = QString::number(cv::mean(brisque.Predict(img))[0]);
					usedTime1 += cv::getTickCount() - timeBegin;
					qDebug() << filename << "\t" << results[i] << "\t"
						<< (cv::getTickCount() - timeBegin) * 1000 / cv::getTickFrequency() << "ms" << endl;
				}
				else
					qDebug() << filename << "\t" << "[Error]" << endl;				
			}
			//=====计算CrystalSet形式=====
			for (auto i = 0; i < fileAmount; i++)
			{
				auto filename = filenames[i];
				if (results[i].isEmpty() == true 
					&& QFileInfo(filename).suffix().toLower()=="crystalset")
				{
					qDebug() << filename;
					CrystalSetManager manager;
					CV_Assert( manager.Load(filename.toStdString()));
					qDebug() << QStringLiteral("  包含图片：") << manager.size();

					QFile f(QFileInfo(output).dir().filePath(
						QFileInfo(output).baseName() + "_" + QFileInfo(filename).baseName() + ".txt"));
					CV_Assert(f.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text));
										
					QTextStream ts(&f);
					auto vccs = manager.Read(0);
					std::function<QString(const CommonCrystalSet& ccs)>fmap = 
						[&brisque](const CommonCrystalSet& ccs)->QString
					{
						QString result;
						for (auto k = 0; k < ccs.size(); k++)
						{
							auto img = const_cast<CommonCrystalSet&>(ccs)[k].Image();							
							if (img.empty() == false)
							{
								auto score = brisque.Predict(img);
								if (score.empty() == false)
									result += QString::number(cv::mean(score)[0]);
								else
									qDebug() << "Error in" << QString::fromStdString(ccs.path()) << k;
							}
							result += ',';
						}
						const_cast<CommonCrystalSet&>(ccs).ReleaseImg();
						if (result.isEmpty() == false && result.back() == ',')result.chop(1);
						return result;
					};
					//这里的编译器类型自动推导有问题，fmap必须先显式声明，reduce不能写成lambda
					//此外，需要使用迭代器形式而非直接使用vccs，因为CommonCrystalSet没有默认的构造和复制函数
					//在surface pro 4上测试，17367图片耗时299s，提速约10倍
					auto future = QtConcurrent::mappedReduced(vccs.begin(), vccs.end(),
						fmap, Predict_CrystalSet_Reduce, QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce);
					
					while (future.isRunning() == true)
					{
						cout << future.progressValue() << '/' << future.progressMaximum() << endl;
						QThread::msleep(1000);
					}

					auto rsl = future.result();
					for (auto j = 0; j < rsl.size(); j++)
						ts << j << ',' << rsl[j] << endl;
// 					for (auto j = 0; j < vccs.size(); j++)
// 					{
// 						if (j % 100 == 0)
// 							std::cout << j << '/' << manager.size() << endl;;
// 						auto &ccs = vccs[j];
// 						for (auto k = 0; k < ccs.size(); k++)
// 						{
// 							auto img = ccs[k].Image();
// 							auto score = brisque.Predict(img);
// 							if (score.empty() == false)
// 								ts << j << '_' << k << ',' << cv::mean(score)[0] << endl;
// 							else
// 								ts << j << '_' << k << ',' << endl;
// 						}
// 						ccs.ReleaseImg();
// 					}
					f.close();
					results[i] = f.fileName();
				}
			}
			//写入output
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
#ifndef _DEBUG
	catch (cv::Exception &e)
	{
		system("pause");
		return -1;
	}
#endif // _DEBUG



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

const char *CONST_CHAR_ModifyDmosPythonContent()
{
return
"import os\n\
\n\
dir = os.path.abspath(os.path.curdir)\n\
f = open(os.path.join(dir, 'dmos.txt'), 'r')\n\
content = []\n\
if f:\n\
    for line in f:\n\
        sep = '/'\n\
        if line.count('\\\\') > 0:\n\
            sep = '\\\\'\n\
        content.append(os.path.join(dir, line.split(sep)[-1]))\n\
    f.close()\n\
\n\
if len(content)>0:\n\
    f = open(os.path.join(dir, 'dmos.txt'), 'w')\n\
    f.writelines(content)\n\
    f.close()";
}

const char * CONST_CHAR_CommanLineParserAboutMessage()
{
	return
		"\nBrisque No-reference Image Quality Assessment v1.0\n"
		"Brisque 无参考图像质量评估 v1.0\n\n"
		"当前共有4种模式：\n"
		"    train-训练模式;generate-训练集生成模式;update-升级旧有文件格式;predict(默认)-预测模式\n"
		"命令行信息包含4个占位参数：mode setting input output，同一参数包含多个值使用逗号分隔，详细定义如下：\n"
		"1.train模式\n"
		"   参考命令行：train 2 -train_expand_sample=8 \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\BRISQUE_2020 - 02 - 10\\dmos.txt\" \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\BRISQUE_2020 - 02 - 10\"\n"
		"   setting:重新训练等级\n"
		"           0.无需训练;1.使用已有feature和dmos;2.重新获得feature，使用已有dmos\n"
		"           3.使用已有的feature，重新获得dmos；；4.重新获得feature和dmos\n"
		"   input:输入文件路径，逗号分隔，最好使用双引号括起\n"
		"           序号0：主观评分dmos文件路径。每个图片应当占据一行，逗号分隔\n"
		"                 第0位为路径，其后若干个数据为主观评分。评分个数为0将被视为无效数据）\n"
		"           序号1~：特征矩阵文件路径，一般存储为32位浮点exr文件\n"
		"                 特征矩阵高度应与dmos个数一致，在1.0版本，宽度为36。一行全0则视为无效数据\n"
		"   output:输出文件夹路径【不是文件路径】\n"
		"                 生成两个json文件。brisque.json为设定文件，brisque_model.json为SVM训练参数\n"
		"                 参考predict模式-setting部分\n"
		"   train_expand_sample:样本扩展参数，见参数说明部分\n"
		"2.generate模式:\n"
		"   参考命令行：generate 1000 \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\first.crystalset\" \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\BRISQUE_2020 - 02 - 10\"\n"
		"	setting:训练集大小\n"
		"           值为生成训练集大小，将从输入数据集中随机抽取\n"
		"   input:输入文件路径【当前只支持单个crystalset数据集】\n"
		"           crystalset文件可通过图像分割算法获得vector<CommonCrystalSet获得，或通过update mode获得\n"
		"           其详细定义参考Crystal.h\n"
		"   output:输出文件夹路径【不是文件路径】\n"
		"                 将在该文件夹下生成对应的晶体图片、不带评分的dmos文件和一个重命名用的python文件\n"
		"                 对应的晶体图片：将对小图片进行适量放大。其名称为源文件名+晶体在图片中序号+png\n"
		"                 不带评分的dmos文件：参见train模式-input部分\n"
		"                 重命名用的python文件：移动了文件夹时使用，用于将dmos当中的图像文件夹路径更新为当前文件夹\n"
		"   generate_reset:是否清空输出文件夹下已有数据，见参数说明部分\n"
		"3.update模式\n"
		"   参考命令行：update csv \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\olddata\\Crystals_Image_Path_1.csv\",\"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\olddata\\Crystals_Info_1.csv\",\"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\fig_batch1" "d:\\Users\\yyx11\\Desktop\\Saved Pictures\\first.crystalset\"\n"
		"	setting:升级类型\n"
		"           暂时只支持CSV转换为crystalset，\n"
		"   input:输入文件路径，逗号分隔\n"
		"   output:输出文件路径\n"
		"4.predict模式\n"
		"   参考命令行：predict \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\brisque.json\" \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\Crystal_   0.png\",\"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\Crystal_   1.png\",\"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\FileNames.txt\",\"d:\\Users\\yyx11\\Desktop\\Saved Pictures\"  \"d:\\Users\\yyx11\\Desktop\\Saved Pictures\\result.txt\"\n"
		"	尝试载入setting。载入失败则返回-1，载入成功则可用于预测ImagePath\n"
		"	setting:setting是一个OpenCV的FileStorage，提供相关信息。\n"
		"	        支持xml, yaml, json，默认名brisque.json，包含：\n"
		"	        lower_range：模型归一化时下限，数组形式\n"
		"	        upper_range：模型归一化时上限，数组形式\n"
		"	        model_path：表示svm模型文件，默认名brisque_model.json\n"
		"	        is_trained：表示是否训练，可以手动改为0从而强制在训练模式中重新训练\n"
		"	input\n"
		"		该参数当中不能出现空格，如果出现空格必须用双引号括起\n"
		"		多个文件以逗号分隔，处理方式如下\n"
		"		i.空则为当前目录下所有支持图片(jpg, png, bmp, jpeg)\n"
		"		ii.目录遍历给定目录下所有支持图片(jpg, png, bmp, jpeg)\n"
		"		iii.txt则按行遍历其中所有文件名\n"
		"		iv.xml、yaml、json会尝试按照CrystalSet的格式进行读取\n"
		"		v.除上述情况外，则遍历给定文件名并输出（如果某个读取失败，对应位置值为 - 1）\n"
		"		vi.注意，不支持递归搜索。即如果在txt中还有另外的txt和文件夹，将预测失败\n"
		"	output\n"
		"		输出为csv文件，格式：路径，分数\n"
		"		如果是crystalset，则会单独生成一个结果文件。格式：路径，结果文件路径\n"
		"		如果识别错误，则分数为空\n"
		"		i.非空则输出到指定文件中，无论什么扩展名均以CSV格式保存。格式：文件名, 评分\n"
		"		ii.空则只输出到控制台\n"
		;
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