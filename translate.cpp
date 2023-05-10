#include "translate.h"

#include <string>
#include <filesystem>

#include "widgets.h"

std::string extrude_file_name_from_path(std::string path)
{
	path = path.substr(path.find_last_of("/\\") + 1);
	typename std::string::size_type const p(path.find_last_of('.'));
	std::string with_extension = (p > 0 && p != std::string::npos ? path.substr(0, p) : path);
	return with_extension.substr(0, with_extension.find_last_of('.'));
}

USL_Translator::Data load_external_file_callback_function(int file_type, std::string library_name, 
	std::vector<std::string>& libraries_path, std::map<std::string, std::vector<uint8_t>>& loaded_libraries)
{
	std::string file_type_str;
	switch (file_type)
	{
	case 0:  file_type_str = "library"; break;
	default: file_type_str = "invalid"; break;
	}

	if (file_type_str == "invalid")
	{
		std::cout << "Error: Requested file type doesn't exsist.\n";
		return { nullptr, 0 };
	}

	std::cout << "Loading " + file_type_str + ": " + library_name + '\n';

	//Check if lib is actually loaded
	if (loaded_libraries.find(library_name) != loaded_libraries.end())
		return { (void*)&loaded_libraries.at(library_name).front(), loaded_libraries.at(library_name).size() };
	//Try to load lib
	else
	{
		for (auto& libs_path : libraries_path)
		{
			auto iterator = std::filesystem::directory_iterator(libs_path);

			for (const auto& file : iterator)
			{
				if (extrude_file_name_from_path(file.path().string()) == library_name)
				{
					std::ifstream library(file.path(), std::fstream::in | std::fstream::out | std::ios::binary);

					loaded_libraries.insert({ library_name, {} });

					//Dump lib content into loaded_libraries
					//Average size of lib is really small, something like 1kb so it isn't too bad
					library.seekg(0, std::ios::end);
					loaded_libraries.at(library_name).resize(library.tellg());
					library.seekg(0, std::ios::beg);
					library.read((char*)&(loaded_libraries.at(library_name)[0]), loaded_libraries.at(library_name).size());
					library.close();

					return { &loaded_libraries.at(library_name)[0], loaded_libraries.at(library_name).size() };
				}
			}
		}
	}
		
	std::cout << "Error: Requested file not found.\n";

	return { nullptr, 0 };
}

std::string gen_header(std::map<std::string, USL_Translator::TranslationResult::HeaderEntry>& data)
{
	auto should_be_put_in_quote = [](std::string txt) -> bool
	{
		if (txt == "true")
			return false;
		if (txt == "false")
			return false;
		bool only_digits = true;
		for (auto& x : txt)
			if (x < '0' || x > '9')
				only_digits = false;
		return !only_digits;
	};

	//add "" if content should be in ""
	auto quote = [should_be_put_in_quote](std::string txt) -> std::string
	{
		if (should_be_put_in_quote(txt))
			return std::string('\"' + txt + '\"');
		else return txt;
	};

	std::string content = "{\n";
	int counter = 0;
	for (auto& entry : data)
	{
		content += '\t' + quote(entry.first) + ':';
		switch (entry.second.type)
		{
		case USL_Translator::TranslationResult::HeaderEntry::Type::Value:
			content += quote(entry.second.content.at(0)); 
			break;
		case USL_Translator::TranslationResult::HeaderEntry::Type::Array:
		{
			content += "[\n";
			for (int i = 0; i < entry.second.content.size(); i++)
			{
				content += "\t\t" + quote(entry.second.content.at(i));
				if (i != entry.second.content.size() - 1)
					content += ',';
				content += '\n';
			}
			content += "\t]";
			break;
		}
		case USL_Translator::TranslationResult::HeaderEntry::Type::Object:
		{
			content += "{\n";
			for (int i = 0; i < entry.second.content.size(); i += 2)
			{
				content += "\t\t" + quote(entry.second.content.at(i)) + ':' + quote(entry.second.content.at(i + 1));
				if (i + 1 != entry.second.content.size() - 1)
					content += ',';
				content += '\n';
			}
			content += "\t}";
			break;
		}
		}
		++counter;
		if (counter != data.size())
			content += ',';
		content += '\n';
	}
	content += '}';
	return content;
}

void Translate(std::vector<std::string>& source_paths, std::string& output_dir,
	std::string file_extension, std::string headers_dir,
	std::vector<std::string>& libraries_path, std::pair<std::string, std::string> from_to)
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
			std::map<std::string, std::vector<uint8_t>> loaded_libraries;

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

				auto LEFCF_Wraper = [&libraries_path, &loaded_libraries](int ft, std::string fn) -> USL_Translator::Data
				{
					return load_external_file_callback_function(ft, fn, libraries_path, loaded_libraries);
				};

				auto result = USL_Translator::USL_Translator::Translate(from_to.first.c_str(), from_to.second.c_str(),
					{ (char*)code.c_str(), code.size() * sizeof(char) }, LEFCF_Wraper);

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
					std::string translated_file_name = output_dir + "\\" + extrude_file_name_from_path(source_file_name) + '.' + file_extension;
					translated.open(translated_file_name, std::ios::binary | std::ios::out);
					translated.write((char*)&result.data[0], result.data.size());
					translated.close();

					std::cout << "Saved: " << translated_file_name << '\n';
					std::cout << "File traslation: SUCCESS\n";

					//Generate header file
					if (result.data_for_header.size() != 0 && headers_dir.size() != 0)
					{
						std::fstream header;
						std::string header_path_and_name = headers_dir + "\\" + extrude_file_name_from_path(source_file_name) + '.' + "json";
						std::cout << "Generating Header: " + header_path_and_name + '\n';
						header.open(header_path_and_name, std::ios::binary | std::ios::out);
						if (header.good())
						{
							auto header_content = gen_header(result.data_for_header);
							header.write(header_content.c_str(), header_content.size());
							header.close();
							std::cout << "Header generation: SUCCESS\n";
						}
						else
						{
							std::cout << "Header generation: FAILED\n";
						}
					}
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