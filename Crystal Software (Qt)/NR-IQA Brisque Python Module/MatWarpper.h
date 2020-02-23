#pragma once
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
	int64_t row = -1, col = -1, channel = -1, type = -1,
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

namespace pybind11 {
	namespace detail {
		//! 实现 cv::Point 和 tuple(x,y) 之间的转换。
		template<>
		struct type_caster<cv::Point> {

			//! 定义 cv::Point 类型名为 tuple_xy, 并声明类型为 cv::Point 的局部变量 value。
			PYBIND11_TYPE_CASTER(cv::Point, _("tuple_xy"));

			//! 步骤1：从 Python 转换到 C++。    
			//! 将 Python tuple 对象转换为 C++ cv::Point 类型, 转换失败则返回 false。    
			//! 其中参数2表示是否隐式类型转换。   
			bool load(handle obj, bool) {
				// 确保传参是 tuple 类型        
				if (!py::isinstance<py::tuple>(obj)) {
					std::logic_error("Point(x,y) should be a tuple!");
					return false;
				}

				// 从 handle 提取 tuple 对象，确保其长度是2。        
				py::tuple pt = reinterpret_borrow<py::tuple>(obj);
				if (pt.size() != 2) {
					std::logic_error("Point(x,y) tuple should be size of 2");
					return false;
				}

				//! 将长度为2的 tuple 转换为 cv::Point。        
				value = cv::Point(pt[0].cast<int>(), pt[1].cast<int>());
				return true;
			}

			//! 步骤2： 从 C++ 转换到 Python。    
			//! 将 C++ cv::Mat 对象转换到 tuple，参数2和参数3常忽略。    
			static handle cast(const cv::Point& pt, return_value_policy, handle) {
				return py::make_tuple(pt.x, pt.y).release();
			}
		};

		template<>
		struct type_caster<cv::Rect> {

			PYBIND11_TYPE_CASTER(cv::Rect, _("tuple_xywh"));

			bool load(handle obj, bool) {
				if (!py::isinstance<py::tuple>(obj)) {
					std::logic_error("Rect should be a tuple!");
					return false;
				}

				py::tuple rect = reinterpret_borrow<py::tuple>(obj);
				if (rect.size() != 4) {
					std::logic_error("Rect (x,y,w,h) tuple should be size of 4");
					return false;
				}

				value = cv::Rect(rect[0].cast<int>(), rect[1].cast<int>(), rect[2].cast<int>(), rect[3].cast<int>());
				return true;
			}

			static handle cast(const cv::Rect& rect, return_value_policy, handle) {
				return py::make_tuple(rect.x, rect.y, rect.width, rect.height).release();
			}
		};
		template<>
		struct type_caster<cv::Mat> {
		public:

			PYBIND11_TYPE_CASTER(cv::Mat, _("numpy.ndarray"));

			//! 1. cast numpy.ndarray to cv::Mat    
			bool load(handle obj, bool) {
				array b = reinterpret_borrow<array>(obj);
				buffer_info info = b.request();

				int nh = 1;
				int nw = 1;
				int nc = 1;
				int ndims = info.ndim;
				if (ndims == 2) {
					nh = info.shape[0];
					nw = info.shape[1];
				}
				else if (ndims == 3) {
					nh = info.shape[0];
					nw = info.shape[1];
					nc = info.shape[2];
				}
				else {
					throw std::logic_error("Only support 2d, 3d matrix");
					return false;
				}

				int dtype;
				if (info.format == format_descriptor<unsigned char>::format()) {
					dtype = CV_8UC(nc);
				}
				else if (info.format == format_descriptor<int>::format()) {
					dtype = CV_32SC(nc);
				}
				else if (info.format == format_descriptor<float>::format()) {
					dtype = CV_32FC(nc);
				}
				else {
					throw std::logic_error("Unsupported type, only support uchar, int32, float");
					return false;
				}
				value = cv::Mat(nh, nw, dtype, info.ptr);
				return true;
			}

			//! 2. cast cv::Mat to numpy.ndarray    
			static handle cast(const cv::Mat& mat, return_value_policy, handle defval) {
				std::string format = format_descriptor<unsigned char>::format();
				size_t elemsize = sizeof(unsigned char);
				int nw = mat.cols;
				int nh = mat.rows;
				int nc = mat.channels();
				int depth = mat.depth();
				int type = mat.type();
				int dim = (depth == type) ? 2 : 3;
				if (depth == CV_8U) {
					format = format_descriptor<unsigned char>::format();
					elemsize = sizeof(unsigned char);
				}
				else if (depth == CV_32S) {
					format = format_descriptor<int>::format();
					elemsize = sizeof(int);
				}
				else if (depth == CV_32F) {
					format = format_descriptor<float>::format();
					elemsize = sizeof(float);
				}
				else {
					throw std::logic_error("Unsupport type, only support uchar, int32, float");
				}

				std::vector<size_t> bufferdim;
				std::vector<size_t> strides;
				if (dim == 2) {
					bufferdim = { (size_t)nh, (size_t)nw };
					strides = { elemsize * (size_t)nw, elemsize };
				}
				else if (dim == 3) {
					bufferdim = { (size_t)nh, (size_t)nw, (size_t)nc };
					strides = { (size_t)elemsize * nw * nc, (size_t)elemsize * nc, (size_t)elemsize };
				}
				return array(buffer_info(mat.data, elemsize, format, dim, bufferdim, strides)).release();
			}
		};
	}
} //!  end namespace pybind11::detail

namespace std
{
	std::string to_string(const cv::Rect&rect)
	{
		return std::string("[") + std::to_string(rect.width) + " x " + std::to_string(rect.height)
			+ " from (" + std::to_string(rect.x) + ", " + std::to_string(rect.y) + ")]";
	}
}