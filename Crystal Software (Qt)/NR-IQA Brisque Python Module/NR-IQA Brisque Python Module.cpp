// NR-IQA Brisque Python Module.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "NR-IQA Brisque Python Module.h"
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <opencv2/core/mat.hpp>

namespace py = pybind11;

cv::Mat Numpy2Mat(py::array& input)
{
#ifdef _DEBUG
	py::print(input.ndim(), input.shape()[0], input.shape()[1], input.shape()[2],
		input.dtype(), input.dtype().kind(),input.dtype().itemsize());
#endif // _DEBUG
	//根据input.shape()和input.dtype()确定类型
	int row = -1, col = -1, channel = -1, type = -1,
		ndim = input.ndim(), itemsize = input.dtype().itemsize();
	if (ndim == 2 || ndim == 3)
	{
		row = input.shape()[0];
		col = input.shape()[1];
		if (ndim == 3)
			channel = input.shape()[2];
		else
			channel = 1;
		switch (input.dtype().kind())
		{
		case 'u'://无符号整型
			if(itemsize==1)type = CV_8UC(channel);
			else if(itemsize==2)type = CV_16UC(channel);
			break;
		case 'f'://浮点型
			if (itemsize == 4)type = CV_32FC(channel);
			else if (itemsize == 8)type = CV_64FC(channel);
			else if (itemsize == 2)type = CV_16FC(channel);
			break;
		case 'i'://有符号整型
			if (itemsize == 4)type = CV_32SC(channel);
			else if (itemsize == 1)type = CV_8SC(channel);
			else if (itemsize == 2)type = CV_16SC(channel);
			break;
		case 'b':
			if (itemsize == 1)type = CV_8UC(channel);
			break;
		default:
			break;
		}
		if(type==-1)
			throw std::runtime_error(std::string("Image dtype [") + input.dtype().kind() + "] is not support");
	}
	else
		throw std::runtime_error(std::string("Image dim [") + std::to_string(ndim) + "] is not support");
	
	py::buffer_info buf = input.request();
	return cv::Mat(row, col, type, buf.ptr)/*.clone()*/;//TODO:这里要注意，不清楚是否需要clone()
}
py::array Mat2Numpy(cv::Mat& input) 
{
	if(input.dims>4)
		throw std::runtime_error(std::string("Image dim [") + std::to_string(input.dims) + "] is not support");
	std::vector<ssize_t>shape;
	for (auto i = 0; i < input.dims; i++)
		shape.push_back(input.size.p[i]);
	py::array dst = py::array(shape, input.data);
	return dst;
}

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
	return Mat2Numpy(m);
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
