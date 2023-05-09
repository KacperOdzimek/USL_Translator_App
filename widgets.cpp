#include "widgets.h"
#include <cstdlib>

void Clear()
{
#ifdef _WIN32
	std::system("cls");
#else
	// Assume POSIX
	std::system("clear");
#endif
}

void Line()
{
	std::cout << "-----------------------------------------------------------\n";
}

void Choice(std::vector<std::pair<std::string, std::function<void()>>> choices)
{
	int i = 1;
	for (auto& option : choices)
	{
		std::cout << '[' << i << ']' << option.first << '\n';
		i++;
	}
	int choice = -1;
	while (true)
	{
		std::cout << "===> ";
		std::string choice_as_str;
		std::cin >> choice_as_str;
		choice = atoi(choice_as_str.c_str());
		if (choice <= choices.size() && choice > 0)
		{
			Line();
			break;
		}
		std::cout << "Invalid option!\n";
	}

	choices.at(choice - 1).second();
}
