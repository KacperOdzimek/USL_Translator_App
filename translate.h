#pragma once
#include "common.h"

void Translate(std::vector<std::string>& source_paths, std::string& output_dir, std::string file_extension,
	std::vector<std::string>& libraries_path, std::pair<std::string, std::string> from_to);