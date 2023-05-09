#include "pages.h"
#include "widgets.h"

#include <string>

#include "translate.h"

void MainMenu()
{
	Line();
	Choice(
		{
			{"Translate",						PageTranslate},
			{"Create blank compiling pipeline", CreatePipeline},
			{"Edit compiling pipeline",			EditPipeline},
			{"Exit",							[]() {return;}}
		}
	);
}

std::vector<std::string> Split(std::string src, std::string delimiter)
{
	std::vector<std::string> result;
	size_t last = 0; size_t next = 0;
	while ((next = src.find(delimiter, last)) != std::string::npos)
	{
		result.push_back(src.substr(last + (last == 0 ? 0 : 1), next - last - (last == 0 ? 0 : 1)));
		last = next + 1;;
	} 
	result.push_back(src.substr(last + 1));
	return result;
}

void PageTranslate()
{
	std::string config_file_path;
	std::ifstream config_file;

	while (true)
	{
		std::cout << "Pipeline path: ";
		std::cin >> config_file_path;
		config_file.open(config_file_path, std::fstream::in | std::fstream::out);
		if (config_file.good())
			break;
		else
		{
			Line();
			std::cout << "No such file\n";
			Line();
		}
	}

	std::vector<std::string> program;

	std::string line;
	while (std::getline(config_file, line))
	{
		program.push_back(line);
	}

	std::vector<std::string> SourcePaths;
	std::vector<std::string> LibsPaths;
	std::string OutputDir;

	for (auto& program_line : program)
	{
		auto splited = Split(program_line, "<<");

		if (program_line == "Program:")
			continue;
		else if (splited[0] == "AddSourcePath")
			SourcePaths.push_back(splited[1]);
		else if (splited[0] == "AddLibsPath")
			LibsPaths.push_back(splited[1]);
		else if (splited[0] == "SetOutputDir")
			OutputDir = splited[1];
		else if (splited[0] == "Translate")
		{
			Translate(SourcePaths, OutputDir, LibsPaths, {splited[1], splited[2]});
		}
	}
	config_file.close();

	MainMenu();
}

void CreatePipeline()
{
	std::string name;
	std::string path;
	std::ofstream config_file;

	while (true)
	{
		std::cout << "Pipeline name: ";
		std::cin >> name;
		std::cout << "Where to put config file? (absolute path): ";
		std::cin >> path;

		auto path_name_extension = path + '\\' + name + ".utpc";

		config_file.open(path_name_extension);

		if (config_file.good())
			break;
		else
		{
			Line();
			std::cout << "Invalid path\n";
			Line();
		}
	}

	config_file << "Program:\n";

	config_file.close();

	Line();
	std::cout << "Pipeline Created\n";
	MainMenu();
}

void EditPipeline()
{
	std::ofstream config_file;
	std::string path;

	while (true)
	{
		std::cout << "Pipeline path: ";
		std::cin >> path;
		config_file.open(path, std::ios::in);
		if (config_file.good())
			break;
		else
		{
			Line();
			std::cout << "No such file\n";
			Line();
		}
	}

	config_file << "Program:\n";
	
	std::vector<std::string> program;

	//Name of command, arguments names
	std::map<std::string, std::vector<std::string>> commands = 
	{
		{"AddSourcePath", {"Path"}},
		{"AddLibsPath",   {"Path"}},
		{"SetOutputDir",  {"Dir"}},
		{"Translate",     {"From", "To"}},
	};

	while (true)
	{
		std::string line;
		std::cout << "Command: ";
		std::cin >> line;
		if (line == "exit")
			break;
		bool found_matching = false;
		for (auto& command : commands)
		{
			if (line == command.first)
			{
				found_matching = true;
				std::vector<std::string> args;
				for (auto& argument : command.second)
				{
					std::cout << ("\t[" + argument + "]: ");
					std::string arg;
					std::cin >> arg;
					args.push_back(arg);
				}
				std::string result = line;
				for (auto& arg : args)
					result += "<<" + arg;
				program.push_back(result);
			}
		}
		if (found_matching == false)
			std::cout << "Invalid command\n";
	}

	for (auto& l : program)
		config_file << l << '\n';

	config_file.close();

	MainMenu();
}