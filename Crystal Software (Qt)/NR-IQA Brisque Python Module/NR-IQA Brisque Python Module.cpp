// NR-IQA Brisque Python Module.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "NR-IQA Brisque Python Module.h"
#include <iostream>

#include "MatWarpper.h"
#include "..\NR-IQA Brisque\Crystal.h"

#include <opencv2/highgui/highgui.hpp>

void testim(py::array& input)
{
	auto m = Numpy2Mat(input);
	cv::imshow("", m);
	cv::waitKey();
}
#include <opencv2\imgproc.hpp>
py::array testreturn(py::array& input)
{
	auto m = Numpy2Mat(input);
	cv::cvtColor(m, m, cv::COLOR_BGR2GRAY);
	cv::threshold(m, m, 0, 255, cv::THRESH_OTSU | cv::THRESH_BINARY);
	return Mat2Numpy(m({ cv::Range(100,400),cv::Range(100,400) }));
}

PYBIND11_MODULE(PyCrystal, m) {

	m.doc() = "pybind11 example module";

	// Add bindings here
	m.def("foo", []() {
		return "Hello, World!";
	});
	m.def("testim", &testim);
	using namespace pybind11::literals;//用来支持_a
	m.def("testreturn", &testreturn,"test return","img"_a);//默认值写作"a"_a=1
	
	py::class_<Crystal>(m, NAME2STR(Crystal))
		.def(py::init<>())
		.def(py::init<const cv::Mat&, const contour_t&>())
		.def("Region", &Crystal::Region)
		.def("Contour", py::overload_cast<>(&Crystal::Contour))
		.def("Contour",py::overload_cast<>(&Crystal::Contour,py::const_))
		.def("Image", [](Crystal&c)->cv::Mat {return c.Image().clone(); })//&Crystal::Image)
		.def("__repr__", [](Crystal &c)
		{
			return std::string("<PyCrystal.Crystal of Region ") + std::to_string(c.Region());
		});

	py::class_<CommonCrystalSet>(m, NAME2STR(CommonCrystalSet))
		.def(py::init<std::string>())
		//.def(py::init<CommonCrystalSet&&>())
		.def("path", &CommonCrystalSet::path)
		.def("__len__", &CommonCrystalSet::size)//[](const CommonCrystalSet&ccs) {return ccs.size(); })
		.def("get", [](CommonCrystalSet&ccs, size_t id)->Crystal& {return ccs[id]; })
		.def("GetContour", py::overload_cast<int>(&CommonCrystalSet::GetContour))
		.def("GetContour", py::overload_cast<int>(&CommonCrystalSet::GetContour, py::const_))
		.def("PushBack", &CommonCrystalSet::PushBack, "contour"_a, "isLoadImage"_a)
		.def("OriginImage", [](CommonCrystalSet&ccs)->cv::Mat {return ccs.OriginImg()->clone(); })//TODO:进一步研究智能指针处理，以免除复制开销
		.def("ReleaseImg",&CommonCrystalSet::ReleaseImg)
		.def("__repr__", [](const CommonCrystalSet &ccs)
		{
		return std::string("<PyCrystal.CommonCrystalSet of Image ") + ccs.path()
			+ " with [" + std::to_string(ccs.size()) + "] crystals";
		});

	py::class_<CrystalSetManager>(m, NAME2STR(CrystalSetManager))
		.def(py::init<>())
		.def("Load", &CrystalSetManager::Load,"path"_a)
		.def("Clear", &CrystalSetManager::Clear)
		.def("Read", py::overload_cast<size_t,size_t>(&CrystalSetManager::Read),"begin"_a,"end"_a= 999999999)
		.def("Get", &CrystalSetManager::Get)
		.def("Seek", &CrystalSetManager::Seek,"idx"_a)
		.def("Pos", &CrystalSetManager::Pos)
		.def("IsOpen", &CrystalSetManager::IsOpen)
		.def("IsEnd", &CrystalSetManager::IsEnd)
		.def("GetDir", &CrystalSetManager::GetDir)
		.def("SetDir", &CrystalSetManager::SetDir,"newdir"_a)
		.def("__len__", &CrystalSetManager::size);
}
