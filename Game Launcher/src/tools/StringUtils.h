#pragma once
#include "../framework.h"

// �����ַ��ַ���(UTF-16)ת��ΪUTF-8����ı�׼�ַ���
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// ��UTF-8����ı�׼�ַ���ת��Ϊ���ַ��ַ���(UTF-16)
inline std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}