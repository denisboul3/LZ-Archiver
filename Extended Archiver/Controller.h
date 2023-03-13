#pragma once
#include <map>
#include <string>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/crc.h>
#include <cryptopp/osrng.h>
#include <iostream>
#include <cstdarg>

namespace ArchiveStructs
{
	typedef struct SArchiveEntry
	{
		unsigned int fileLength;
		unsigned int offset;
		unsigned int size;
		char name[1024];
	} TArchiveEntry;

	struct FileCompile
	{
		std::vector<unsigned char> data;

		unsigned int compiledFileLength;
		unsigned int fileLength;
	};

	struct ArchiverConfig
	{
		std::string extension;
		bool caseSensitive;
		//std::string salt;
		unsigned int magic_key;
		bool debug;
		std::string unpack_pref;
		bool showhex;
	};
};

class NewFileSystem
{
	public:
		bool CreateArchive(const char* szArchiveName, const char* path);
		ArchiveStructs::FileCompile CompileFile(std::string input_src);

		bool UnpackArchive(const char* szArchiveName);
		//void LoadKeyList(const char* szArchiveName);

		void LoadConfig();

		std::string GetExtension()
		{
			return config.extension;
		}

	private:
		ArchiveStructs::ArchiverConfig config;
//		std::map<std::string, std::string> keyList;

};