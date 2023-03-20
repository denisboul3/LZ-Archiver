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
	else if (argc == 4) {
		//.exe read bla.testy file.py
		if (strcmp(argv[1], "read") == 0) {
		
			std::string file2read = argv[2];
			archiver.UnpackArchive(file2read.c_str(), true);
			const void* data = (const void*)archiver.RawData(argv[3]);
			std::cout << (const char*)data;
			//std::copy(data, std::ostream_iterator<unsigned char>(std::cout));
		}
	}
	return 0;
}
