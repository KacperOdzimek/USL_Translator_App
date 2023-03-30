#include "translate.h"

#include <string>
#include <filesystem>

#include "widgets.h"

#include "USL_Translator.h"
#include "USL_To_Binary.h"

void Translate(std::vector<std::string>& source_paths, std::string& output_dir, std::pair<std::string, std::string> from_to)
{
	int all_files = 0;
	int failed_tasks = 0;

	for (auto& source : source_paths)
	{
		auto iterator = std::filesystem::directory_iterator(source);

		Line();
		std::cout << "Translating " << from_to.first << " to " << from_to.second << '\n';
		Line();

		for (const auto& entry : iterator)
		{
			std::cout << "Translating file: " << entry.path() << '\n';
			std::ifstream file_to_translate(entry.path(), std::fstream::in | std::fstream::out | std::ios::binary);
			if (file_to_translate)
			{
				++all_files;

				std::string code;

				file_to_translate.seekg(0, std::ios::end);
				code.resize(file_to_translate.tellg());
				file_to_translate.seekg(0, std::ios::beg);
				file_to_translate.read(&code[0], code.size());
				file_to_translate.close();

				auto result = USL_Translator::USL_Translator::Translate(from_to.first.c_str(), from_to.second.c_str(),
					{ (char*)code.c_str(), code.size() * sizeof(char) });

				for (auto& item : result.prompt)
					std::cout << item;

				if (result.success)
				{
					std::fstream translated;
					std::string file_name = output_dir + "\\" + "plik" + ".ub";
					translated.open(file_name, std::ios::binary | std::ios::out);
					for (uint8_t byte : result.data)
						translated.write((char*)&byte, 1);
					translated.close();
					std::cout << "File traslation: SUCCESS\n";
				}
				else
				{
					std::cout << "File traslation: FAILED\n";
					++failed_tasks;
				}
				
			}
			else
			{
				std::cout << "File does not exist / is not accessible\n";
			}
		}
	}
	
	std::cout << "Translation result: \n";
	std::cout << "\tCorrectly translated: " << all_files - failed_tasks << '\n';
	std::cout << "\tFailed to translate: " << failed_tasks << '\n';
	std::cout << "\tAll: " << all_files << '\n';
}