#pragma once
#include "common.h"

void Clear();
void Line();
void Choice(std::vector<std::pair<std::string, std::function<void()>>> choices);