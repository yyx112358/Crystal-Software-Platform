#include "stdafx.h"
#include "Crystal.h"
#include <opencv2/imgproc.hpp>

using namespace std;

Crystal::Crystal(const cv::Mat& image, const contour_t& in_contour)
	:_contour(in_contour), _originImage(std::make_shared<cv::Mat>(image))
{
	
}

cv::Rect Crystal::Region()
{
	if (_originRegion.width == -1)
		_originRegion = cv::boundingRect(Contour());
	return _originRegion;
}

std::shared_ptr<cv::Mat> CommonCrystalSet::OriginImg()
{
	if (_originImage == nullptr || _originImage->empty() == true)
		_originImage = std::make_shared<cv::Mat>(cv::imread(_imgPath));
	return _originImage;
}

#include <fstream>
const uint32_t CrystalSetManager::PROGRAM_VERSION = 0x000001;
const size_t CrystalSetManager::FPTR_WIDTH = 9;
int myitoa(unsigned long num, char*buf)
{
	auto oldbuf = buf;
	while (num > 0) 
	{
		*buf++ = num % 10 + '0';
		num /= 10;
	}
	reverse(oldbuf, buf);
	return buf - oldbuf;
}
//读取到下一个逗号位置（包括逗号）
char* myGetCommaSegment(FILE*f, char*buf)
{
	auto i = 0u;
	char c = '\0';
	while (c != '\n'&&c != ','&&feof(f) == 0)
	{
		fread(&c, 1, 1, f);
		buf[i++] = c;
	}
	buf[i-1] = '\0';
	return buf;
}

bool CrystalSetManager::Load(std::string path)
{
	Clear();
	f = fopen(path.c_str(), "rb");
	if (f == nullptr)
		return false;
	
	char fbuf[0xFFFF];
	//文件头
	fptr = fread(fbuf, 1, sizeof("<Crystal Set Text>\n")-1, f);
	if (memcmp(fbuf, "<Crystal Set Text>\n", sizeof("<Crystal Set Text>\n")-1) != 0) 
	{
		fclose(f);
		return false;
	}
	version = atoi(myGetCommaSegment(f, fbuf));
	_dir = std::string(myGetCommaSegment(f, fbuf));
	size_t sz = atoi(myGetCommaSegment(f, fbuf));
	myGetCommaSegment(f, fbuf);

	fptr = ftell(f);
	//建立快速查找表（在surface pro 4上，DEBUG模式，17367节点，24MB下，DEBUG模式只需要190ms，效费比很高）
	for (auto i = 0; i < sz && feof(f) == 0; i++)
	{
		//检查fptr
		CV_Assert(fptr == ftell(f));
		_searchTbl.push_back(fptr);
		//获取next_fptr
		fread(fbuf, 1, FPTR_WIDTH + 1, f);
		next_fptr = atoi(fbuf);
		if (next_fptr <= fptr)
			break;
		//将文件指针指向下一个节点开头
		fread(fbuf, 1, next_fptr - fptr - FPTR_WIDTH - 1, f);
		fptr = next_fptr;
	}
	
	return true;
}

std::vector<CommonCrystalSet> CrystalSetManager::Read(size_t begin, size_t end /*= 999999999*/)
{
	std::vector<CommonCrystalSet>vccs;
	if (f == nullptr)
		return vccs;

	return vccs;
}
/*
格式：
第一行：<Crystal Set Text>
第二行：版本号，图像文件夹路径，总个数（9位），保留
下一集合指针（9位），上一集合指针（9位），[集合序号，]晶体个数，保留，保留，文件名（不含文件夹）
晶体轮廓
*/
bool CrystalSetManager::SaveAll(const std::vector<CommonCrystalSet>&vccs, std::string filename
	, std::string dir /*=""*/, size_t begin /*= 0*/, size_t end /*= 999999999*/)
{
#define FPTR_WIDTH 9
#if FPTR_WIDTH != 9
#error Please modify the sprintf() in function CrystalSetManager::SaveAll()
#endif
#undef FPTR_WIDTH
	auto tmpfilename = filename + ".tmp";
	FILE*f = fopen(tmpfilename.c_str(), "wb+");//创建临时文件。写入成功后，将在函数末尾替换原文件
	if (f == nullptr)
		return false;

	long fptr = 0, prev_fptr = 0;
	char buf[0xFFFF];
	//写入文件头
	auto offset = fprintf_s(f, "<Crystal Set Text>\n"
		"%6d,%s,%9zd,\n",
		CrystalSetManager::PROGRAM_VERSION, dir.c_str(), vccs.size());
	//写入数据
	fptr = ftell(f);
	for (auto i=begin;i<vccs.size()&&i<end;i++)
	{
		const auto &ccs = vccs[i];
		//写入CommonCrystalSet信息，注意这里的fptr（指向下一个节点起始位置）还不正确，因为此时本节点长度未知
		auto cnt = sprintf_s<sizeof(buf)>(buf, "%9d,%9d,%9d,%4zd,%s\n", fptr, prev_fptr, i, 
			ccs.size(), ccs.path().c_str());
		prev_fptr = fptr;
		for (auto j = 0u; j < ccs.size(); j++)
		{
			auto&ct = ccs.GetContour(j);
			for (auto k = 0u; k < ct.size() - 1; k++)
			{
				cnt += myitoa(ct[k].x, buf+cnt);
				buf[cnt++] = ',';
				cnt += myitoa(ct[k].y, buf+cnt);
				buf[cnt++] = ',';
			}
			buf[cnt - 1] = '\n';
		}
		CV_Assert(cnt <= sizeof(buf));
		buf[cnt] = '\0';
		
		//此时长度信息已知，重新修改fptr
		fptr += cnt;
		char fptrbuf[FPTR_WIDTH+1];
		sprintf_s<sizeof(fptrbuf)>(fptrbuf, "%9zd", fptr);
		memcpy(buf, fptrbuf, FPTR_WIDTH);
		fwrite(buf, 1, cnt, f);
	}
	fseek(f, offset, SEEK_SET);

	fclose(f);
	remove(filename.c_str());
	rename(tmpfilename.c_str(), filename.c_str());
	return true;
}

bool CrystalSetManager::Update(std::string oldpath)
{
	return false;
}

bool CrystalSetManager::UpdateCSV(std::string pathfile, std::string infofile, std::string outfile, std::string dir)
{
	fstream ifsCts(infofile, ios::in);
	fstream	ifsNames(pathfile, ios::in);
	if (ifsCts.is_open() == false || ifsNames.is_open() == false)
	{
		cout << "打开文件失败" << endl;
		ifsCts.close();
		ifsNames.close();
		return false;
	}
	std::string s;
	vector<CommonCrystalSet>vccs;
	while (getline(ifsNames, s))//每一行是一个源文件名
		vccs.push_back(CommonCrystalSet(s));
	ifsNames.close();

	int imageIdx = 0;
	while (getline(ifsCts, s))//源文件序号，轮廓
	{
		istringstream iss(s);
		int tmp;
		iss >> tmp;
		if (tmp != imageIdx)
		{
			vccs[imageIdx].ReleaseImg();
			imageIdx = tmp;
			if (imageIdx % 100 == 0)cout << imageIdx << ',';
		}

		contour_t ct;
		vector<int>vi;
		while (iss >> tmp)
			vi.push_back(tmp);
		if (vi.size() % 2 != 0)
			continue;
		for (auto i = 0; i < vi.size(); i += 2)
			ct.push_back(cv::Point(vi[i], vi[i + 1]));
		vccs[imageIdx].PushBack(ct, false);
	}
	ifsCts.close();
	cout << "ok" << endl;

	return CrystalSetManager::SaveAll(vccs, outfile,dir);
}
