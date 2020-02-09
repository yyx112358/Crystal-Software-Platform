#pragma once

#include <opencv2\core\mat.hpp>
//#include <opencv2\core\persistence.hpp>
//#include <opencv2\imgproc\imgproc.hpp>
#include <vector>
#include <memory>

typedef std::vector<cv::Point>contour_t;
#define NAME2STR(name) #name
#define PRINT_PARAM(param)	<<NAME2STR(param)<<param


class Crystal
{
public:
	Crystal(){}
	Crystal(const cv::Mat& image, const contour_t& in_contour);
	Crystal(std::shared_ptr<cv::Mat>originImage, const contour_t& in_contour)
		:_contour(in_contour), _originImage(originImage) {}
	Crystal(const Crystal&src)
		:_contour(src._contour), _originRegion(src._originRegion), _originImage(src._originImage){}
	Crystal(const Crystal&&src)
		:_contour(std::move(src._contour)), _originRegion(std::move(src._originRegion)), _originImage(std::move(src._originImage)) {}
	~Crystal(){}

	cv::Rect Region();
	contour_t& Contour() { return _contour; }
	const contour_t& Contour()const { return _contour; }
	cv::Mat Image() { return (*_originImage)(Region()); }//晶体的原始图像
private:
	cv::Rect _originRegion = cv::Rect(0, 0, -1, -1);//在原始图像中位置
	contour_t _contour;//原始轮廓
	std::shared_ptr<cv::Mat>_originImage;//原始图像。理论上，Mat本身就具有隐式共享的特征，但为了保证线程安全性使用智能指针
};

//从属于同一张图像的晶体集合
//共享同一张图像，从而减少内存占用
//	图像惰性加载
class CommonCrystalSet
{
public:
	CommonCrystalSet(std::string inputPath) :_imgPath(inputPath) {}
	CommonCrystalSet(cv::Mat originImg,std::string outputPath="");
	CommonCrystalSet(CommonCrystalSet&& another)
		:_imgPath(std::move(another._imgPath)),_crystals(std::move(another._crystals)),
		_originImage(std::move(another._originImage)) {}
	
	std::string path()const { return _imgPath; }
	size_t size()const { return _crystals.size(); }
	Crystal&operator[](int id);

	const contour_t& GetContour(int id)const { return _crystals[id].Contour(); }
	void PushBack(contour_t ct ,bool isLoadImage=true) { _crystals.push_back(Crystal(_originImage, ct)); }

	std::shared_ptr<cv::Mat> OriginImg();
	void ReleaseImg() { if(_originImage) _originImage->release(); }

protected:
	std::string _imgPath;
	std::vector<Crystal>_crystals;
	std::shared_ptr<cv::Mat>_originImage;//原始图像。理论上，Mat本身就具有隐式共享的特征，但为了保证线程安全性使用智能指针
};

#include <stdio.h>
//基本思想：文本文件，顺序读写，末尾插入，按行处理，逗号分隔，链式连接
//	不追求极致性能优化，string等随意使用。预计规模30000图片，100 0000晶体，约100M文本文件
//	原则上，这个类是只读的。而static函数Save()和Update()负责写
class CrystalSetManager
{
public:
	CrystalSetManager() { Clear(); }
	~CrystalSetManager() { if(f) fclose(f); }

	bool Load(std::string path);
	void Clear() {
		_dir.clear(); _searchTbl.clear(); version = 0; if (f) fclose(f);
		fptr = 0; next_fptr = 0; _nodeIdx = 0;
	}

	std::vector<CommonCrystalSet>Read(size_t begin, size_t end = 999999999);
	CommonCrystalSet Get();//获取一个节点，并将指针移到下一个节点
	CommonCrystalSet Get(size_t idx);
	bool Seek();//将指针移到下一个节点
	bool Seek(size_t idx);//将指针移到第idx个节点，不反序列化
	size_t Pos()const;

	static bool SaveAll(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		std::string dir ="", size_t begin = 0, size_t end = 999999999);
	static bool SaveAppend(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		size_t begin = 0, size_t end = 999999999);
	static bool SaveSieve(const std::vector<CommonCrystalSet>&vccs, std::string filename, 
		std::function<bool(const CommonCrystalSet&)>f);
	static bool Update(std::string oldpath);
	static bool UpdateCSV(std::string pathfile, std::string infofile, std::string outfile,std::string dir);

	std::string _dir;
	std::vector<size_t>_searchTbl;

	const static uint32_t PROGRAM_VERSION;
	const static size_t FPTR_WIDTH;
	uint32_t version;
private:
	FILE*f = nullptr;//打开的文件
	size_t fptr = 0, next_fptr = 0;//用于遍历的文件指针
	size_t _nodeIdx = 0;
};
