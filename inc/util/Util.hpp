#pragma once

#include <vector>
#include <string>

#include <api/IComm.hpp>
#include <data/User.hpp>

namespace util {
	template<typename _Tp>
	_Tp *nonNull(_Tp *ptr) {
		if (!ptr) throw std::runtime_error("ptr null");
		return ptr;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector() {
		return std::vector<_Tp>();
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0) {
		std::vector<_Tp> vec;

		vec.push_back(a0);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2, _Tp a3) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);
		vec.push_back(a3);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2, _Tp a3, _Tp a4) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);
		vec.push_back(a3);
		vec.push_back(a4);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2, _Tp a3, _Tp a4, _Tp a5) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);
		vec.push_back(a3);
		vec.push_back(a4);
		vec.push_back(a5);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2, _Tp a3, _Tp a4, _Tp a5, _Tp a6) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);
		vec.push_back(a3);
		vec.push_back(a4);
		vec.push_back(a5);
		vec.push_back(a6);

		return vec;
	}

	template<typename _Tp>
	std::vector<_Tp> makeVector(_Tp a0, _Tp a1, _Tp a2, _Tp a3, _Tp a4, _Tp a5, _Tp a6, _Tp a7) {
		std::vector<_Tp> vec;

		vec.push_back(a0);
		vec.push_back(a1);
		vec.push_back(a2);
		vec.push_back(a3);
		vec.push_back(a4);
		vec.push_back(a5);
		vec.push_back(a6);
		vec.push_back(a7);

		return vec;
	}

	std::vector<std::string> parseList(std::string list);
} // namespace util
