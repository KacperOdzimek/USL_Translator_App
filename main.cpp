#include "common.h"
#include "widgets.h"
#include "pages.h"

#include "USL_Translator.h"
#include "USL_To_Binary.h"

int main()
{
	USL_Translator::USL_Translator::Initialize();

	USL_Translator::TranslatorBase* NewTranslator = new USL_Translator::USL_To_Binary;
	USL_Translator::USL_Translator::Load(NewTranslator);

	NewTranslator = new USL_Translator::USL_Binary_To_GLSL;
	USL_Translator::USL_Translator::Load(NewTranslator);

	Line();
	std::cout << "\t\tWelcome to USL translator!\n";
	MainMenu();
	
	USL_Translator::USL_Translator::Terminate();

	return 0;
}