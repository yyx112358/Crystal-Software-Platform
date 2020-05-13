#pragma once

#include <opencv2\core\mat.hpp>
#include <vector>
#include <functional>
#include <chrono> 

typedef std::vector<cv::Point>contour_t;
#define NAME2STR(name) #name
#define PRINT_PARAM(param)	<<NAME2STR(param)<<param


class Crystal
{
public:
	friend class CommonCrystalSet;

	Crystal(){}
	Crystal(const cv::Mat& image, const contour_t& in_contour);
	Crystal(std::shared_ptr<cv::Mat>originImage, const contour_t& in_contour)
		:_contour(in_contour), _originImage(originImage) {}
	Crystal(std::shared_ptr<cv::Mat>originImage, contour_t&& in_contour)
		:_contour(std::move(in_contour)), _originImage(originImage) {}
	Crystal(const Crystal&src)
		:_contour(src._contour), _originRegion(src._originRegion), _originImage(src._originImage){}
	Crystal(const Crystal&&src)
		:_contour(std::move(src._contour)), _originRegion(std::move(src._originRegion)), _originImage(std::move(src._originImage)) {}
	~Crystal(){}

	cv::Rect Region();
	contour_t& Contour() { return _contour; }
	const contour_t& Contour()const { return _contour; }
	cv::Mat Image() 
	{ 
		if (_originImage.lock() != nullptr)
			return (*_originImage.lock())(Region());
		else
			return cv::Mat();
	}//晶体的原始图像
	friend std::string to_string(Crystal&);
private:
	cv::Rect _originRegion = cv::Rect(0, 0, -1, -1);//在原始图像中位置
	contour_t _contour;//原始轮廓
	std::weak_ptr<cv::Mat>_originImage;//原始图像。理论上，Mat本身就具有隐式共享的特征，但为了保证线程安全性使用智能指针
};

//从属于同一张图像的晶体集合
//共享同一张图像，从而减少内存占用
//	图像惰性加载：初始化时不会读取图像只是保存其路径，第一次调用OriginImg()或operator []时加载，ReleaseImg()释放图像
class CommonCrystalSet
{
public:
	friend class CrystalSetManager;

	CommonCrystalSet(std::string inputPath) :_imgPath(inputPath) {}
	CommonCrystalSet(cv::Mat originImg,std::string outputPath="");
	CommonCrystalSet(CommonCrystalSet&& another)
		:_imgPath(std::move(another._imgPath)),_crystals(std::move(another._crystals)),
		_originImage(std::move(another._originImage)) {}
	CommonCrystalSet(const CommonCrystalSet& another)//TODO:不确定这样做对不对（将const T&传递给std::move())
		:_imgPath(std::move(another._imgPath)), _crystals(std::move(another._crystals)),
		_originImage(std::move(another._originImage)) {}
	
	std::string path()const { return _imgPath; }
	size_t size()const { return _crystals.size(); }
	Crystal&operator[](size_t id);//重载运算符，获取一个晶体。【注：会调用OriginImg()，可能会读取图片】

	std::vector<contour_t>GetContours()const { std::vector<contour_t>cts; cts.reserve(size()); for (const auto&c : _crystals)cts.push_back(c.Contour()); return cts; }
	contour_t& GetContour(size_t id) { CV_Assert(id < size()); return _crystals[id].Contour(); }
	const contour_t& GetContour(size_t id)const { CV_Assert(id < size()); return _crystals[id].Contour(); }
	void PushBack(const contour_t& ct ,bool isLoadImage=true) { _crystals.push_back(Crystal(_originImage, ct)); }

	std::shared_ptr<cv::Mat> OriginImg();
	void ReleaseImg() { _originImage = nullptr;/*if(_originImage) _originImage->release();*/ }

	std::chrono::system_clock::time_point GetCaptureTime()const;//尝试分析文件路径，获取拍摄时间，分析失败返回0
	cv::Mat DrawCrystals();
protected:
	std::string _imgPath;
	std::vector<Crystal>_crystals;
	std::shared_ptr<cv::Mat>_originImage;//原始图像。理论上，Mat本身就具有隐式共享的特征，但为了保证线程安全性使用智能指针
};

#include <stdio.h>
//基本思想：文本文件，顺序读写，末尾插入，按行处理，逗号分隔，链式连接
//	不追求极致性能优化，string等随意使用。预计规模30000图片，100 0000晶体，约100M文本文件
//	原则上，这个类是只读的。而static函数Save()和Update()负责写。不建议多线程操作，既不安全，也不效率
//	扩展名crystalset
//  性能测试：Release模式下只需要800ms（文件大小24MB，含17367集合和530000晶体，使用surface pro 4测试）
class CrystalSetManager
{
public:
	CrystalSetManager() { Clear(); }
	~CrystalSetManager() { if(f) fclose(f); }

	bool Load(std::string path);
	void Clear() {
		_dir.clear(); _searchTbl.clear(); version = 0; if (IsOpen()) fclose(f);
		fptr = 0; next_fptr = 0; _nodeIdx = 0;
	}

	std::vector<CommonCrystalSet>Read(size_t begin, size_t end = 999999999);//读取[begin,end)范围节点
	std::vector<CommonCrystalSet>Read(std::function<bool(const CommonCrystalSet&)>filterFunction,
		size_t begin, size_t end = 999999999);//读取[begin,end)范围内，且符合filterFunction的节点
	CommonCrystalSet Get();//获取一个节点，并将指针移到下一个节点
	bool Seek(size_t idx);//将指针移到第idx个节点，不反序列化
	size_t Tell()const { return _nodeIdx; }//获取下一个要读取的节点序号

	static bool SaveAll(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		std::string dir ="", size_t begin = 0, size_t end = 999999999);
	static bool SaveAppend(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		size_t begin = 0, size_t end = 999999999);
	static bool SaveSieve(const std::vector<CommonCrystalSet>&vccs, std::string filename, 
		std::function<bool(const CommonCrystalSet&)>f);
	static bool UpdateDir(std::string path,std::string newdir);
	static bool UpdateCSV(std::string pathfile, std::string infofile, std::string outfile,std::string dir);
	
	//TODO:二分搜索得到处于一定时间范围之内的图片

	size_t Pos() const { return _nodeIdx; }
	bool IsOpen()const { return f != nullptr; }
	bool IsEnd()const { return _nodeIdx >= size(); }
	size_t size()const { return _searchTbl.size(); }
	std::string GetDir()const { return _dir; }
	void SetDir(std::string newDir) { _dir = newDir; }

	const static uint32_t PROGRAM_VERSION;
	const static size_t FPTR_WIDTH;
private:
	uint32_t version;

	std::string _dir;
	std::vector<size_t>_searchTbl;//查找表，用于快速索引

	FILE*f = nullptr;//打开的文件
	size_t fptr = 0, next_fptr = 0;//用于遍历的文件指针
	size_t _nodeIdx = 0;//下一个要读取的节点序号
};
