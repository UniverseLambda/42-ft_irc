#pragma once

#define FT_INTERFACE_PRELUDE(X) X(); X(const X &); virtual ~X(); X &operator=(const X &);
#define FT_INTERFACE_PRELUDE_IMPL(X) X::X() {} X::X(const X &) {} X::~X() {} X &X::operator=(const X &) { return *this; }
