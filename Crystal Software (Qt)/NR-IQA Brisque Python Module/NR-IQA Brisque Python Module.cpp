// NR-IQA Brisque Python Module.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "NR-IQA Brisque Python Module.h"
#include <iostream>

#include "MatWarpper.h"
#include "..\NR-IQA Brisque\Crystal.h"
#include "..\NR-IQA Brisque\Brisque.h"


PYBIND11_MODULE(PyCrystal, m) {

	m.doc() = "pybind11 example module";

	// Add bindings here
	using namespace pybind11::literals;//用来支持_a
	
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

	py::class_<Brisque>(m, NAME2STR(Brisque))
		.def(py::init<>())
		.def("Load", &Brisque::Load, "settingPath"_a)
		.def("Save", &Brisque::Save, "settingPath"_a = ".\\brisque.json", "modelPath"_a = ".\\brisque_model.json", py::const_)
		.def("Clear", &Brisque::Clear)
		.def("Train", py::overload_cast<const cv::Mat&, const cv::Mat&>(&Brisque::Train), "featureMat"_a, "dmosMat"_a)
		.def("Train", py::overload_cast<std::string, std::string>(&Brisque::Train), "featurePath"_a, "dmosPath"_a)
		.def("Train", py::overload_cast<std::vector<std::string>, std::string>(&Brisque::Train), "featurePaths"_a, "dmosPath"_a)
		.def("Predict", py::overload_cast<cv::InputArray>(&Brisque::Train), "img"_a, py::const_)
		.def("Predict", py::overload_cast<std::string>(&Brisque::Train), "filename"_a, py::const_)
		.def("isTrained", &Brisque::isTrained, py::const_);

}
