// NR-IQA Brisque Python Module.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "NR-IQA Brisque Python Module.h"
#include <iostream>

#include "MatWarpper.h"

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
	m.def("testreturn", &testreturn);
}
