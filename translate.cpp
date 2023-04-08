#include "translate.h"

#include <string>
#include <filesystem>

#include "widgets.h"

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
					std::cout << item << '\n';

				if (result.success)
				{
					std::string source_file_name{ entry.path().u8string() };
					int slash_pos = source_file_name.size() - 1;

					while (source_file_name.at(slash_pos) != '\\')
						slash_pos--;

					source_file_name = source_file_name.substr(slash_pos + 1, source_file_name.size() - slash_pos);

					//Dump to file
					std::fstream translated;
					std::string translated_file_name = output_dir + "\\" + source_file_name + ".trs";
					translated.open(translated_file_name, std::ios::binary | std::ios::out);
					translated.write((char*)&result.data[0], result.data.size());
					translated.close();

					std::cout << "Saved: " << translated_file_name << '\n';
					std::cout << "File traslation: SUCCESS\n";
				}
				else
				{
					std::cout << "File traslation: FAILED\n";
					++failed_tasks;
				}
			}
			else if (!std::filesystem::is_directory(entry.path()))
			{
				std::cout << "File does not exist / is not accessible\n";
			}
			else continue;
			Line();
		}

		source_paths.clear();
		output_dir = "";
	}
	
	std::cout << "Translation result: \n";
	std::cout << "\tCorrectly translated: " << all_files - failed_tasks << '\n';
	std::cout << "\tFailed to translate: " << failed_tasks << '\n';
	std::cout << "\tAll: " << all_files << '\n';
}