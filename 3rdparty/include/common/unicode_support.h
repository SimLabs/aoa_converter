/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <stdexcept>
#include <string>
#include <boost/scoped_array.hpp>

#include "encodings/utf8.h"

namespace unicode
{

inline std::string utf16to8(std::wstring const& str)
{
   std::string res;
   utf8::utf16to8(str.begin(), str.end(), std::back_inserter(res));

   return res;
}

inline std::wstring utf8to16(std::string const& str)
{
   std::wstring res;
   utf8::utf8to16(str.begin(), str.end(), std::back_inserter(res));

   return res;
}

inline std::string w2a(std::wstring const& p)
{
   size_t const s = p.size() + 1;
   size_t const n = wcstombs(NULL, p.c_str(), s) + 1;
   if (n == 0)
      throw std::runtime_error("invalid wide character");

   boost::scoped_array<char> buf(new char[n]);
   size_t const r = wcstombs(buf.get(), p.c_str(), s);
   Assert(r + 1 == n);

   buf.get()[n - 1] = '\0';
   return buf.get();
}

inline std::wstring a2w(std::string const& p)
{
   size_t const s = p.size() + 1;
   size_t const n = mbstowcs(NULL, p.c_str(), s) + 1;
   if (n == 0)
      throw std::runtime_error("invalid multi-byte character");

   boost::scoped_array<wchar_t> buf(new wchar_t[n]);
   size_t const r = mbstowcs(buf.get(), p.c_str(), s);
   Assert(r + 1 == n);

   buf.get()[n - 1] = L'\0';
   return buf.get();
}

} //unicode
