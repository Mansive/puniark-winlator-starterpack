#pragma once

#include <string>

void InitOutput();
void CloseOutput();
void OutLine(const std::wstring &line);
void ErrLine(const std::wstring &line);
void DebugLine(bool enabled, const std::wstring &line);
