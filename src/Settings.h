#pragma once

typedef void(__stdcall* NV)(char* name, char* value, void* lpParam);

bool Ini_IsValidSettings(char* name, char* value);

char* Ini_Read(const wchar_t* szFileName);
void Ini_Parse(char* str, NV NameValueCallback, void* lpParam);
