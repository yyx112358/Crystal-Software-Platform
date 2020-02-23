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
	//����input.shape()��input.dtype()ȷ������
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
		case 'u'://�޷�������
			if (itemsize == 1)type = CV_8UC(channel);
			else if (itemsize == 2)type = CV_16UC(channel);
			break;
		case 'f'://������
			if (itemsize == 4)type = CV_32FC(channel);
			else if (itemsize == 8)type = CV_64FC(channel);
			else if (itemsize == 2)type = CV_16FC(channel);
			break;
		case 'i'://�з�������
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
	return cv::Mat(row, col, type, buf.ptr)/*.clone()*/;//TODO:����Ҫע�⣬������Ƿ���Ҫclone()
}

py::array Mat2Numpy(const cv::Mat& input)
{
	if (input.dims > 4)
		throw std::runtime_error(std::string("Image dim [") + std::to_string(input.dims) + "] is not support");
	std::vector<ssize_t>shape;
	for (auto i = 0; i < input.dims; i++)
		shape.push_back(input.size.p[i]);

	//����Ƿ������洢�������input����һ��ͼ���ROI�����ⲿ���ݹ����ԭ��ʹ�����ݲ�����������Ҫ����clone()
	cv::Mat newimg = (input.isContinuous()) ? (input) : (input.clone());
	
	//�����������
	switch (newimg.type() & (~CV_DEPTH_MAX))
	{
	case CV_8UC1:return py::array(shape, reinterpret_cast<uint8_t*>(newimg.data));//ǿ��ת���Ա�array��ģ������ƶ�
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
		//! ʵ�� cv::Point �� tuple(x,y) ֮���ת����
		template<>
		struct type_caster<cv::Point> {

			//! ���� cv::Point ������Ϊ tuple_xy, ����������Ϊ cv::Point �ľֲ����� value��
			PYBIND11_TYPE_CASTER(cv::Point, _("tuple_xy"));

			//! ����1���� Python ת���� C++��    
			//! �� Python tuple ����ת��Ϊ C++ cv::Point ����, ת��ʧ���򷵻� false��    
			//! ���в���2��ʾ�Ƿ���ʽ����ת����   
			bool load(handle obj, bool) {
				// ȷ�������� tuple ����        
				if (!py::isinstance<py::tuple>(obj)) {
					std::logic_error("Point(x,y) should be a tuple!");
					return false;
				}

				// �� handle ��ȡ tuple ����ȷ���䳤����2��        
				py::tuple pt = reinterpret_borrow<py::tuple>(obj);
				if (pt.size() != 2) {
					std::logic_error("Point(x,y) tuple should be size of 2");
					return false;
				}

				//! ������Ϊ2�� tuple ת��Ϊ cv::Point��        
				value = cv::Point(pt[0].cast<int>(), pt[1].cast<int>());
				return true;
			}

			//! ����2�� �� C++ ת���� Python��    
			//! �� C++ cv::Mat ����ת���� tuple������2�Ͳ���3�����ԡ�    
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