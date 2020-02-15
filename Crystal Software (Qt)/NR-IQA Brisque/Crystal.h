#pragma once

//#include <opencv2\core\mat.hpp>
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
	}//�����ԭʼͼ��
private:
	cv::Rect _originRegion = cv::Rect(0, 0, -1, -1);//��ԭʼͼ����λ��
	contour_t _contour;//ԭʼ����
	std::weak_ptr<cv::Mat>_originImage;//ԭʼͼ�������ϣ�Mat����;�����ʽ�������������Ϊ�˱�֤�̰߳�ȫ��ʹ������ָ��
};

//������ͬһ��ͼ��ľ��弯��
//����ͬһ��ͼ�񣬴Ӷ������ڴ�ռ��
//	ͼ����Լ���
class CommonCrystalSet
{
public:
	friend class CrystalSetManager;

	CommonCrystalSet(std::string inputPath) :_imgPath(inputPath) {}
	CommonCrystalSet(cv::Mat originImg,std::string outputPath="");
	CommonCrystalSet(CommonCrystalSet&& another)
		:_imgPath(std::move(another._imgPath)),_crystals(std::move(another._crystals)),
		_originImage(std::move(another._originImage)) {}
	
	std::string path()const { return _imgPath; }
	size_t size()const { return _crystals.size(); }
	Crystal&operator[](size_t id);

	contour_t& GetContour(int id) { return _crystals[id].Contour(); }
	const contour_t& GetContour(int id)const { return _crystals[id].Contour(); }
	void PushBack(const contour_t& ct ,bool isLoadImage=true) { _crystals.push_back(Crystal(_originImage, ct)); }

	std::shared_ptr<cv::Mat> OriginImg();
	void ReleaseImg() { _originImage = nullptr;/*if(_originImage) _originImage->release();*/ }

protected:
	std::string _imgPath;
	std::vector<Crystal>_crystals;
	std::shared_ptr<cv::Mat>_originImage;//ԭʼͼ�������ϣ�Mat����;�����ʽ�������������Ϊ�˱�֤�̰߳�ȫ��ʹ������ָ��
};

#include <stdio.h>
//����˼�룺�ı��ļ���˳���д��ĩβ���룬���д������ŷָ�����ʽ����
//	��׷���������Ż���string������ʹ�á�Ԥ�ƹ�ģ30000ͼƬ��100 0000���壬Լ100M�ı��ļ�
//	ԭ���ϣ��������ֻ���ġ���static����Save()��Update()����д����������̲߳������Ȳ���ȫ��Ҳ��Ч��
//	��չ��crystalset
//  ���ܲ��ԣ�Releaseģʽ�¶�ȡ��ʱ800ms���ң��ļ���С24MB����17367���Ϻ�530000���壬ʹ��surface pro 4���ԣ�
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

	std::vector<CommonCrystalSet>Read(size_t begin, size_t end = 999999999);
	std::vector<CommonCrystalSet>Read(std::function<bool(const CommonCrystalSet&)>filterFunction,
		size_t begin, size_t end = 999999999);
	CommonCrystalSet Get();//��ȡһ���ڵ㣬����ָ���Ƶ���һ���ڵ�
	bool Seek(size_t idx);//��ָ���Ƶ���idx���ڵ㣬�������л�


	static bool SaveAll(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		std::string dir ="", size_t begin = 0, size_t end = 999999999);
	static bool SaveAppend(const std::vector<CommonCrystalSet>&vccs, std::string filename,
		size_t begin = 0, size_t end = 999999999);
	static bool SaveSieve(const std::vector<CommonCrystalSet>&vccs, std::string filename, 
		std::function<bool(const CommonCrystalSet&)>f);
	static bool UpdateDir(std::string path,std::string newdir);
	static bool UpdateCSV(std::string pathfile, std::string infofile, std::string outfile,std::string dir);

	size_t Pos() const { return _nodeIdx; }
	bool IsOpen()const { return f != nullptr; }
	bool IsEnd()const { return _nodeIdx >= size(); }
	size_t size()const { return _searchTbl.size(); }

	const static uint32_t PROGRAM_VERSION;
	const static size_t FPTR_WIDTH;	
private:
	uint32_t version;

	std::string _dir;
	std::vector<size_t>_searchTbl;//���ұ����ڿ�������

	FILE*f = nullptr;//�򿪵��ļ�
	size_t fptr = 0, next_fptr = 0;//���ڱ������ļ�ָ��
	size_t _nodeIdx = 0;
};
