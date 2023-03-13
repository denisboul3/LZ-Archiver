#include <Windows.h>
#include "Controller.h"
#include <fstream>
#include <assert.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <filesystem>

int main(int argc, const char* argv[])
{
	auto archiver = NewFileSystem();
	archiver.LoadConfig();
	if (argc == 2)
	{
		if (std::filesystem::is_directory(argv[1]))
		{
			std::string szArchive = argv[1];
			szArchive += archiver.GetExtension();
			archiver.CreateArchive(szArchive.c_str(), argv[1]);
		}
		else
		{
			std::string fName = argv[1];
			archiver.UnpackArchive(fName.c_str());
		}
	}
	return 0;
}
