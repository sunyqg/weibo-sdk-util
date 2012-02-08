#if defined (_WIN32)
#   include "stdafx.h"
#endif //_WIN32

#include <util/common/StringUtil.hxx>

#include <iostream>  
#include <sstream>
#include <iterator>

using namespace Util;

const string StringUtil::sEmpty = "";

#if defined(_WIN32) || defined(WIN32)
const std::wstring StringUtil::sWEmpty = L"";
#endif