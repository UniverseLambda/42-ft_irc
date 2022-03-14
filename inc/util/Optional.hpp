#pragma once

#include <stdexcept>

namespace util {
	template<typename _Tp>
	class Optional {
	public:
		typedef _Tp type;

	private:
		type mValue;
		bool mIsValid;

	public:
		Optional(): mValue(), mIsValid(false) {}
		Optional(type value): mValue(value), mIsValid(true) {}
		Optional(const Optional &orig): mValue(orig.mValue), mIsValid(orig.mIsValid) {}
		~Optional() {}

		Optional &operator=(const Optional &orig) {
			mValue = orig.mValue;
			mIsValid = orig.mIsValid;

			return (*this);
		}

		type *operator->() {
			return &mValue;
		}

		const type &operator->() const {
			return &mValue;
		}

		type &operator*() {
			return mValue;
		}

		const type &operator*() const {
			return mValue;
		}

		type unwrap() const {
			if (!mIsValid) {
				throw std::runtime_error("unwrap on invalid Optional");
			}
			return mValue;
		}

		operator bool() const {
			return mIsValid;
		}

		bool operator==(const Optional &other) const {
			return (mIsValid == other.mIsValid) && (!mIsValid || (mValue == other.mValue));
		}
	};
} // namespace util
