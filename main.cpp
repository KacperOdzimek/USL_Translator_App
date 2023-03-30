#include "common.h"
#include "widgets.h"
#include "pages.h"

#include "USL_Translator.h"
#include "USL_To_Binary.h"

int main()
{
	USL_Translator::USL_Translator::Initialize();
	USL_Translator::USL_To_Binary* Compiler = new USL_Translator::USL_To_Binary;
	USL_Translator::USL_Translator::Load(Compiler);

	Line();
	std::cout << "\t\tWelcome to USL translator!\n";
	MainMenu();
	
	USL_Translator::USL_Translator::Terminate();

	return 0;
}