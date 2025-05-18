#pragma once
#include "../framework.h"

// ½«¿í×Ö·û×Ö·û´®(UTF-16)×ª»»ÎªUTF-8±àÂëµÄ±ê×¼×Ö·û´®
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// ½«UTF-8±àÂëµÄ±ê×¼×Ö·û´®×ª»»Îª¿í×Ö·û×Ö·û´®(UTF-16)
inline std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}