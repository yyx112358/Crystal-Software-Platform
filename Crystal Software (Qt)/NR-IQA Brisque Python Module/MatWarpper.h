#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <opencv2/core/mat.hpp>

namespace py = pybind11;

cv::Mat Numpy2Mat(py::array& input)
{
#ifdef _DEBUG
	py::print(input.ndim(), input.shape()[0], input.shape()[1], input.shape()[2],
		input.dtype(), input.dtype().kind(), input.dtype().itemsize());
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
			if (itemsize == 1)type = CV_8UC(channel);
			else if (itemsize == 2)type = CV_16UC(channel);
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
		if (type == -1)
			throw std::runtime_error(std::string("Image dtype [") + input.dtype().kind() + "] is not support");
	}
	else
		throw std::runtime_error(std::string("Image dim [") + std::to_string(ndim) + "] is not support");

	py::buffer_info buf = input.request();
	return cv::Mat(row, col, type, buf.ptr)/*.clone()*/;//TODO:这里要注意，不清楚是否需要clone()
}

py::array Mat2Numpy(const cv::Mat& input)
{
	if (input.dims > 4)
		throw std::runtime_error(std::string("Image dim [") + std::to_string(input.dims) + "] is not support");
	std::vector<ssize_t>shape;
	for (auto i = 0; i < input.dims; i++)
		shape.push_back(input.size.p[i]);

	//检测是否连续存储，如果因input是另一张图像的ROI或由外部数据构造等原因使得数据不连续，则需要进行clone()
	cv::Mat newimg = (input.isContinuous()) ? (input) : (input.clone());
	
	//根据情况复制
	switch (newimg.type() & (~CV_DEPTH_MAX))
	{
	case CV_8UC1:return py::array(shape, reinterpret_cast<uint8_t*>(newimg.data));//强制转义以便array做模板参数推断
	case CV_8SC1:return py::array(shape, reinterpret_cast<int8_t*>(newimg.data));
	case CV_16UC1:return py::array(shape, reinterpret_cast<uint16_t*>(newimg.data));
	case CV_16SC1:return py::array(shape, reinterpret_cast<int16_t*>(newimg.data));
	case CV_32SC1:return py::array(shape, reinterpret_cast<int32_t*>(newimg.data));
	case CV_32FC1:return py::array(shape, reinterpret_cast<float*>(newimg.data));
	case CV_64FC1:return py::array(shape, reinterpret_cast<double*>(newimg.data));
	//case CV_16FC1:
	default:
		throw std::runtime_error(std::string("Image type [") + std::to_string(newimg.type()) + "] is not support");
	}
}