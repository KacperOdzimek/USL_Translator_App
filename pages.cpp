#include "pages.h"
#include "widgets.h"

#include <sstream>
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
	std::string HeadersDir;
	std::string FileExtension = "trs";

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
		else if (splited[0] == "SetOutputExtension")
			FileExtension = splited[1];
		else if (splited[0] == "SetHeadersFolderPath")
			HeadersDir = splited[1];
		else if (splited[0] == "Translate")
			Translate(SourcePaths, OutputDir, FileExtension, HeadersDir, LibsPaths, {splited[1], splited[2]});
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
		std::cout << "Where to put the config file? (absolute path): ";
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
	std::fstream config_file;
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

	Clear();
	
	std::vector<std::string> program;

	std::string program_line;
	while (std::getline(config_file, program_line))
		program.push_back(program_line);

	program.erase(program.begin());

	config_file.close();

	//Name of command, arguments names
	std::map<std::string, std::vector<std::string>> commands = 
	{
		{"AddSourcePath",			{"Path"}},
		{"AddLibsPath",				{"Path"}},
		{"SetOutputDir",			{"Directory"}},
		{"SetOutputExtension",		{"Extension"}},
		{"SetHeadersFolderPath",	{"Path"}},
		{"Translate",				{"From", "To"}},
	};

	std::cout << "Welcome to pipeline editor. Use following controls to edit your pipeline:\n"
		<< "AddSourcePath\t\t{Path}\t\t- add source files folder path\n"
		<< "AddLibsPath\t\t{Path}\t\t- add libraries path\n"
		<< "SetOutputDir\t\t{Path}\t\t- set where to save translated files\n"
		<< "SetOutputExtension\t{Extension}\t- set file extension applied to translated files\n"
		<< "SetHeadersFolderPath\t{Path}\t\t- set directory into which shader data will be saved\n"
		<< "Translate\t\t{From} {To}\t- translate all files from one format to another\n"
		<< "Overwrite\t\t{Line}\t\t- overwrite line with new content\n"
		<< "Insert\t\t\t{After}\t\t- insert new command after given line\n"
		<< "Show\t\t\t\t\t- show pipeline content\n"
		<< "Clear\t\t\t\t\t- clear entire pipeline\n"
		<< "Exit\t\t\t\t\t- save and exit\n"
		<< "Abort\t\t\t\t\t- exit without saving\n"
		;

	std::string line;

	auto load_other_command = [&]() {
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
	};

	while (true)
	{
		std::cout << "Command: ";
		std::cin >> line;
		if (line == "Exit")
			break;
		else if (line == "Abort")
		{
			goto end;
		}
		else if (line == "Show")
		{
			int line_id = 1;;
			for (auto& line : program)
			{
				std::cout << '[' << line_id << ']' << line << '\n';
				++line_id;
			}
		}
		else if (line == "Overwrite")
		{
			std::string str;
			int line_id;
			while (true)
			{
				std::cout << "\t[line]: ";
				std::cin >> str;
				line_id = std::atoi(str.c_str());
				if (line_id <= 0 || line_id > program.size())
					std::cout << "\tInvalid line!\n";
				else
					break;
			}
			std::cout << "\t[New Command]: ";
			std::cin >> line;
			load_other_command();
			str = program.back();
			program.erase(program.end() - 1);
			program.at(line_id - 1) = str;
		}
		else if (line == "Insert")
		{
			std::string str;
			int line_id;
			while (true)
			{
				std::cout << "\t[After]: ";
				std::cin >> str;
				line_id = std::atoi(str.c_str());
				if (line_id <= 0 || line_id > program.size())
					std::cout << "\tInvalid line!\n";
				else
					break;
			}
			std::cout << "\t[Command]: ";
			std::cin >> line;
			load_other_command();
			str = program.back();
			program.erase(program.end() - 1);
			program.insert(program.begin() + line_id, str);
		}
		else if (line == "Clear")
		{
			std::cout << "Are you sure? yes/no\n";
			std::string choice;
			std::cin >> choice;
			if (choice == "yes")
				program.clear();
		}
		else
		{
			load_other_command();
		}
	}


	config_file.open(path, std::ios::out);
	config_file.clear();

	config_file << "Program:\n";

	for (auto& line : program)
		config_file << line << '\n';

	config_file.close();

end:
	Clear();
	MainMenu();
}