#pragma once

namespace util {
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
} // namespace util