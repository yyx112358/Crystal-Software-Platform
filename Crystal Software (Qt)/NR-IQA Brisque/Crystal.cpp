#include "stdafx.h"
#include "Crystal.h"
#include <opencv2/imgproc.hpp>
//#include <opencv2\core\persistence.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\imgcodecs.hpp>

#include <iostream>
#include <regex>

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

std::string to_string(Crystal&c)
{
	auto rect = c.Region();
	std::string("Crystal of Region ") + "[" + std::to_string(rect.width) + " x " + std::to_string(rect.height)
		+ " from (" + std::to_string(rect.x) + ", " + std::to_string(rect.y) + ")]";
}

Crystal& CommonCrystalSet::operator[](size_t id)
{
	CV_Assert(id < size());
	if (_originImage == nullptr || _crystals[id]._originImage.expired() == true)
	{
		for (auto &c : _crystals)
			c._originImage = OriginImg();
	}
	return _crystals[id];
}

std::shared_ptr<cv::Mat> CommonCrystalSet::OriginImg()
{
	if (_originImage == nullptr || _originImage->empty() == true) 
	{
		_originImage = std::make_shared<cv::Mat>(cv::imread(_imgPath));
		for (auto &c : _crystals)
			c._originImage = OriginImg();
	}
	return _originImage;
}


std::chrono::system_clock::time_point CommonCrystalSet::GetCaptureTime() const
{
	CV_Assert(0 && "Not Implement");
	//static std::regex re("\\d-\\d-\\d \\dhh \\dmin \\dsec\\dms",std::regex::)
}

cv::Mat CommonCrystalSet::DrawCrystals()
{
	cv::Mat disp;
	if (disp.channels() == 1)
		cv::cvtColor(*OriginImg(), disp, cv::COLOR_GRAY2BGR);
	else
		disp = OriginImg()->clone();

	auto cts = GetContours();
	for (auto i=0;i<cts.size();i++)
	{
		cv::Scalar_<uint8_t> color;
		color.randu(0, 255);
		cv::drawContours(disp, cts, i, color, 3);
		cv::putText(disp, std::to_string(i), cts[i].front(), cv::FONT_HERSHEY_COMPLEX, 1, color, 2);
	}
	return disp;
}

#include <fstream>
const uint32_t CrystalSetManager::PROGRAM_VERSION = 0x000001;
const size_t CrystalSetManager::FPTR_WIDTH = 9;
int myitoa(unsigned long num, char*buf)
{
	auto oldbuf = buf;
	do
	{
		*buf++ = num % 10 + '0';
		num /= 10;
	} while (num > 0);
	reverse(oldbuf, buf);
	return buf - oldbuf;
}
//��ȡ����һ������λ�ã��������ţ�
char* myGetCommaSegment(FILE*f, char*buf)
{
	auto i = 0u;
	char c = '\0';
	while (c != ','&&c != '\n'&&feof(f) == 0)
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
	fopen_s(&f, path.c_str(), "rb");//f = fopen(path.c_str(), "rb");
	
	if (f == nullptr)
		return false;
	
	char fbuf[0xFFFF];
	//�ļ�ͷ
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
	//�������ٲ��ұ���surface pro 4�ϣ�DEBUGģʽ��17367�ڵ㣬24MB�£�DEBUGģʽֻ��Ҫ190ms��Ч�ѱȺܸߣ�
	for (auto i = 0; i < sz && feof(f) == 0; i++)
	{
		//���fptr
		CV_Assert(fptr == ftell(f));
		_searchTbl.push_back(fptr);
		//��ȡnext_fptr
		fread(fbuf, 1, FPTR_WIDTH + 1, f);
		next_fptr = atoi(fbuf);
		if (next_fptr <= fptr)
			break;
		//���ļ�ָ��ָ����һ���ڵ㿪ͷ
		fread(fbuf, 1, next_fptr - fptr - FPTR_WIDTH - 1, f);
		fptr = next_fptr;
	}
	Seek(0);
	return true;
}

std::vector<CommonCrystalSet> CrystalSetManager::Read(size_t begin, size_t end /*= 999999999*/)
{
	std::vector<CommonCrystalSet>vccs;
	if (f == nullptr || begin >= size() || begin >= end)//����ҿ�����
		return vccs;
	if (end > _searchTbl.size())end = size();

	//TODO:�����ļ���ֺͲ��л�
	Seek(begin);
	for (auto i = begin; i < end; i++)
		vccs.push_back(Get());
	return vccs;
}

std::vector<CommonCrystalSet> CrystalSetManager::Read(std::function<bool(const CommonCrystalSet&)>filterFunction, size_t begin, size_t end /*= 999999999*/)
{
	std::vector<CommonCrystalSet>vccs;
	if (f == nullptr || begin >= size() || begin >= end)//����ҿ�����
		return vccs;
	if (end > _searchTbl.size())end = size();

	//TODO:�����ļ���ֺͲ��л�
	Seek(begin);
	for (auto i = begin; i < end; i++) 
	{
		auto &&ccs = Get();
		if (filterFunction(ccs) == true)
			vccs.push_back(ccs);
	}
	return vccs;
}

//��buf+offset��ʼ��ȡ��ֱ��������һ��','��'\n'
//�޸�','��'\n'Ϊ'\0'��������һ���εĿ�ͷ��buf+offset����ָ����һ���ο�ͷ
//ʹ�øú������Լ�csv�ļ��Ķ�ȡ��ÿ����һ��atoi(myGetCommaSegment(buf,offset);���ɶ�ȡһ�����֣��ο�Get()
char* myGetCommaSegment(char*buf,size_t&offset)
{
	auto begin = offset;
	char c;
	do 
	{
		c = buf[offset++];
	} while (c != ','&&c != '\n');
	buf[offset - 1] = '\0';
	return buf + begin;
}
CommonCrystalSet CrystalSetManager::Get()
{
	CV_Assert(IsEnd() == false && IsOpen() == true && ftell(f) == _searchTbl[_nodeIdx]);
	
	char buf[0xFFFF];
	size_t length = 0;
	if (_nodeIdx + 1 < size())
		length = _searchTbl[_nodeIdx + 1] - _searchTbl[_nodeIdx];
	else
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f) - _searchTbl[_nodeIdx];
		fseek(f, static_cast<long>(_searchTbl[_nodeIdx]), SEEK_SET);
	}
	buf[fread(buf, 1, length, f)] = '\0';
	_nodeIdx++;

	//��ȡCommonCrystalSet��Ϣ
	//��һ����ָ�루9λ������һ����ָ�루9λ����������ţ�����������������������ļ����������ļ��У�
	size_t offset = 0;
	myGetCommaSegment(buf, offset);
	myGetCommaSegment(buf, offset);
	auto nodeIdx = atoi(myGetCommaSegment(buf, offset));	
	auto crystalAmount = atoi(myGetCommaSegment(buf, offset));

	//��ȡ��������
	std::string newdir = _dir;
	if (_dir.empty() == false)newdir += '\\';
	CommonCrystalSet ccs(newdir + std::string(myGetCommaSegment(buf, offset)));
	ccs._crystals.reserve(crystalAmount);
	contour_t ct;
	cv::Point pt;
	for (auto oldoffset = offset,commacnt=0ull; offset < length; offset++)
	{
		//���Ĳ�����ct���ڴ���䣬���Ԥ�ȶ�ȡ���Ÿ����Ӷ�Ԥ�ȷ���ɴ�����ʱ��
		commacnt = 0;
		char c;
		while (offset < length && (c = buf[offset++]) != '\n')
			if (c == ',')
				commacnt++;
		ct.reserve((commacnt + 1) / 2);
		offset = oldoffset;

		for (auto i = 0; i < (commacnt + 1) / 2; i++)
		{
			pt.x = atoi(myGetCommaSegment(buf, offset));
			pt.y = atoi(myGetCommaSegment(buf, offset));
			ct.push_back(pt);
		}
		ccs._crystals.push_back(Crystal(ccs._originImage, std::move(ct)));//��ֵ���ü����ڴ��ط���
		oldoffset = offset;
// 		char c = buf[offset];
// 		if (c == ',')
// 		{
// 			buf[offset] = '\0';
// 			commacnt++;
// 			if (commacnt % 2 == 1)
// 				pt.x = atoi(buf + oldoffset);
// 			else 
// 			{
// 				pt.y = atoi(buf + oldoffset);
// 				ct.push_back(pt);//�����Ƶ���ڴ��ط�����Ӱ���ٶȵ�һ�����أ�����ں������reserve��shrink_to_fit
// 			}
// 			oldoffset = offset+1;
// 		}
// 		else if (c == '\n')
// 		{
// 			buf[offset] = '\0';
// 			commacnt = 0;
// 			ccs._crystals.push_back(Crystal(ccs._originImage, std::move(ct)));
// // 			contour_t tmp; tmp.swap(ct);
// // 			ccs.PushBack(tmp, false);
// 			oldoffset = offset+1;
// 		}
	}
	CV_Assert(_nodeIdx - 1 == nodeIdx && crystalAmount == ccs.size());
	
	return ccs;
}

bool CrystalSetManager::Seek(size_t idx)
{
	if (IsOpen() == true && idx < _searchTbl.size())
	{
		_nodeIdx = idx;
		fseek(f, static_cast<long>(_searchTbl[idx]), SEEK_SET);
		return true;
	}
	else
		return false;
}

/*
��ʽ��
��һ�У�<Crystal Set Text>
�ڶ��У��汾�ţ�ͼ���ļ���·�����ܸ�����9λ��������
��һ����ָ�루9λ������һ����ָ�루9λ����[������ţ�]����������������������ļ����������ļ��У�
��������
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
	FILE*f = nullptr;
	fopen_s(&f, tmpfilename.c_str(), "wb+");
	//FILE*f = fopen_s(tmpfilename.c_str(), "wb+");//������ʱ�ļ���д��ɹ��󣬽��ں���ĩβ�滻ԭ�ļ�
	if (f == nullptr)
		return false;

	long fptr = 0, prev_fptr = 0;
	char buf[0xFFFF];
	//д���ļ�ͷ
	auto offset = fprintf_s(f, "<Crystal Set Text>\n"
		"%6d,%s,%9zd,\n",
		CrystalSetManager::PROGRAM_VERSION, dir.c_str(), vccs.size());
	//д������
	fptr = ftell(f);
	for (auto i = begin; i < vccs.size() && i < end; i++)
	{
		const auto &ccs = vccs[i];
		//д��CommonCrystalSet��Ϣ��ע�������fptr��ָ����һ���ڵ���ʼλ�ã�������ȷ����Ϊ��ʱ���ڵ㳤��δ֪
		auto cnt = sprintf_s<sizeof(buf)>(buf, "%9d,%9d,%9zd,%4zd,%s\n", fptr, prev_fptr, i, 
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
		
		//��ʱ������Ϣ��֪�������޸�fptr
		fptr += cnt;
		char fptrbuf[FPTR_WIDTH+1];
		sprintf_s<sizeof(fptrbuf)>(fptrbuf, "%9d", fptr);
		memcpy(buf, fptrbuf, FPTR_WIDTH);
		fwrite(buf, 1, cnt, f);
	}
	fseek(f, offset, SEEK_SET);

	fclose(f);
	remove(filename.c_str());
	rename(tmpfilename.c_str(), filename.c_str());
	return true;
}


bool CrystalSetManager::UpdateDir(std::string path, std::string newdir)
{
	CrystalSetManager manager;
	if (manager.Load(path) == false)
		return false;
	manager._dir = newdir;
	return manager.Load(newdir);
}

bool CrystalSetManager::UpdateCSV(std::string pathfile, std::string infofile, std::string outfile, std::string dir)
{
	fstream ifsCts(infofile, ios::in);
	fstream	ifsNames(pathfile, ios::in);
	if (ifsCts.is_open() == false || ifsNames.is_open() == false)
	{
		cout << "���ļ�ʧ��" << endl;
		ifsCts.close();
		ifsNames.close();
		return false;
	}
	std::string s;
	vector<CommonCrystalSet>vccs;
	while (getline(ifsNames, s))//ÿһ����һ��Դ�ļ���
		vccs.push_back(CommonCrystalSet(s));
	ifsNames.close();

	int imageIdx = 0;
	while (getline(ifsCts, s))//Դ�ļ���ţ�����
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
