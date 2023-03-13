#include "Controller.h"
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stack>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <Shlobj.h>
#include <cstdio>
#include <execution>
#include <filesystem>
#include <random>
#include <chrono> 

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "MemoryMapped.h"
#include "sha256.h"
#include "termcolor.hpp"

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

using namespace std::chrono;
std::mutex mu;
#pragma comment(lib, "cryptopp-static.lib")
#pragma comment(lib, "lz4.lib")

namespace fs = std::filesystem;
using namespace rapidjson;
using namespace termcolor;
using namespace ArchiveStructs;

/*
static unsigned int RandomNumber()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(100000, 999999);
	return distr(gen);
}
*/

void NewFileSystem::LoadConfig()
{
	std::ifstream ifs{ "config.json" };
	if (!ifs.is_open())
	{
		std::cout << "Could not open config.json for reading" << std::endl;
		return;
	}

	IStreamWrapper isw{ ifs };

	Document doc{};
	doc.ParseStream(isw);

	if (doc.HasParseError())
	{
		std::cout << "Parsing error" << std::endl;
		return;
	}

	auto members = {"magic_key", "extension", "case_sensitive", "debug", "unpack_pref", "showhex" };
	for (const auto& member : members)
	{
		if (!doc.HasMember(member))
		{
			std::cout << "Member " << member << " does not exists" << std::endl;
			return;
		}
	}
	config = {
		doc["extension"].GetString(),
		doc["case_sensitive"].GetBool(),
		doc["magic_key"].GetUint(),
		doc["debug"].GetBool(),
		doc["unpack_pref"].GetString(),
		doc["showhex"].GetBool()
	};
}

FileCompile NewFileSystem::CompileFile(std::string input_src)
{
	MemoryMapped data(input_src);
	const char* raw = (const char*) data.getData(); 

	FileCompile ret_vals = { {}, 0, 0 };
	ret_vals.fileLength = data.size();
	ret_vals.data.resize(1024 * 1024 * 64);
	ret_vals.compiledFileLength = LZ4_compressHC(raw, reinterpret_cast<char*>(&ret_vals.data[0]), ret_vals.fileLength);
	if (ret_vals.compiledFileLength >= ret_vals.fileLength)
	{
		memcpy(&ret_vals.data[0], raw, strlen(raw));

		ret_vals.compiledFileLength = ret_vals.fileLength;
	}

	return ret_vals;
}

bool NewFileSystem::CreateArchive(const char* szArchiveName, const char* path)
{
	unsigned int key = config.magic_key;
	HANDLE Archivefile = CreateFileA(szArchiveName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (!Archivefile)
		return false;

	DWORD dwWritten;

	WriteFile(Archivefile, &key, sizeof(key), &dwWritten, 0);

	//std::ofstream infoFile;
	//infoFile.open(/*szArchiveName*/));
	//infoFile << "key:" << randtNum << "\n";

	std::vector<std::string> pFiles;
	std::cout << "Writing magic key to " << green << Archivefile << reset << " until offset " << red << sizeof(key) << reset << std::endl;
	for (const auto& p : std::filesystem::recursive_directory_iterator(path)) {
		if (!std::filesystem::is_directory(p)) {
			pFiles.push_back(p.path().string());
		}
	}
	auto start = high_resolution_clock::now();
	try
	{
		tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());
		tbb::parallel_for(tbb::blocked_range<int>(0, pFiles.size()), [&](tbb::blocked_range<int> r)
		{
			for (int i = r.begin(); i < r.end(); ++i)
			{
				mu.lock();
				std::string filePath = pFiles[i];
				if (!config.caseSensitive)
					transform(filePath.begin(), filePath.end(), filePath.begin(), tolower);

				std::replace(filePath.begin(), filePath.end(), '\\', '/');
				std::string insideName = filePath.substr(strlen(path) + 1, strlen(filePath.c_str()));
				std::string fname = insideName;

				//std::string hash = sha256(config.salt + fname);
				//infoFile << hash.c_str() << ":" << fname.c_str() << "\n";
				FileCompile cInfo = CompileFile(filePath);

				TArchiveEntry entry;

				entry.fileLength = cInfo.fileLength;
				entry.offset = sizeof(entry);
				entry.size = cInfo.compiledFileLength;
				strcpy(entry.name, fname.c_str());
				LARGE_INTEGER m, p;

				m.QuadPart = 0;

				SetFilePointerEx(Archivefile, m, &p, 1);

				entry.offset += p.QuadPart;
				DWORD _dwWritten = 0;
				WriteFile(Archivefile, &entry, sizeof(entry), &_dwWritten, 0);
				WriteFile(Archivefile, &cInfo.data[0], cInfo.compiledFileLength, &_dwWritten, 0);

				if (config.debug) {
					std::cout << "-----------------------------------------------------------------" << std::endl;

					if (config.showhex)
						std::cout << std::hex << std::uppercase;
					__int64 startEntryInfo = p.QuadPart;
					__int64 endEntryInfo = p.QuadPart + sizeof(entry);
					std::cout << "Writing file " << green << entry.name << reset << " into " << yellow << szArchiveName << reset << "\n";
					std::cout << green << entry.name << " information: " << reset << std::endl << "\t -> Offset " << cyan << "0x" << startEntryInfo << reset << " until offset " << cyan << "0x" << endEntryInfo << reset << " contains TArchiveEntry struct" << "\n";
					std::cout << "\t -> Data that is being encrypted is at offset " << cyan << "0x" << endEntryInfo << reset << " until offset " << cyan << "0x" << endEntryInfo + cInfo.compiledFileLength << reset << std::endl;

					std::cout << "\t -> Encrypted content length: " << yellow << cInfo.compiledFileLength << reset << std::endl;
				}

				mu.unlock();
			}
		});
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "An unknown exception was thrown!" << std::endl;
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);

	std::cout << "Time taken by function: " << duration.count() << " seconds" << std::endl;
	//infoFile.close();
	return true;
}

/*
void NewFileSystem::LoadKeyList(const char* szArchiveName)
{
	std::ifstream infile("szArchiveName");
	std::string line;
	while (std::getline(infile, line))
	{
		size_t delim = line.find(':');
		std::string fHash = line.substr(0, delim);
		std::string fName = line.substr(delim + 1);
		keyList[fHash] = fName;
	}
}
*/

bool NewFileSystem::UnpackArchive(const char* szArchiveName)
{
	std::string fileName = szArchiveName;
	size_t extPos = fileName.find('.');
	fileName = fileName.substr(0, extPos);

	//LoadKeyList(szArchiveName);
	//if (keyList.empty())
	//	return false;

	HANDLE LoadFile = CreateFileA(szArchiveName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (!LoadFile)
	{
		std::cout << "Error reading: " << szArchiveName << std::endl;
		return false;
	}

	unsigned int key = -1;
	DWORD dwRead = 0;
	bool res = ReadFile(LoadFile, &key, sizeof(key), &dwRead, 0);
	DWORD size = GetFileSize(LoadFile, 0);
	if (key != config.magic_key)
	{
		std::cout << "Wrong magic key" << std::endl;
		return false;
	}

	unsigned int offset = sizeof(key);
	int cnt = 0;
	std::cout << "Magic key is until offset " << red << offset << reset << std::endl;
	while (offset < size)
	{
		TArchiveEntry entry;
		LARGE_INTEGER m;
		m.QuadPart = offset;
		SetFilePointerEx(LoadFile, m, 0, FILE_BEGIN);
		res = ReadFile(LoadFile, &entry, sizeof(entry), &dwRead, 0);
		offset = entry.offset + entry.size;
		std::vector<unsigned char> tmp(entry.size + entry.fileLength);
		res = ReadFile(LoadFile, &tmp[0], entry.size, &dwRead, 0);
		
		BYTE* pbData = new BYTE[entry.fileLength + 1];

		if (entry.size != entry.fileLength && (LZ4_decompress_fast(reinterpret_cast<char*>(&tmp[0]), reinterpret_cast<char*>(&tmp[entry.size]), entry.fileLength) != entry.size))
		{
			std::cout << "ERROR_DECOMPRESSION_FAILED" << std::endl;
			return false;
		}
		memcpy(pbData, &tmp[entry.size], min(entry.fileLength + 1, entry.fileLength));

		if (config.debug) {
			std::cout << "-----------------------------------------------------------------" << std::endl;
			if (config.showhex) {
				std::cout << std::hex << std::uppercase;
			}
			std::cout << "Start reading from offset " << cyan << "0x" << m.QuadPart << reset << " until offset " << cyan << "0x" << entry.offset + entry.size << reset << std::endl;
			std::cout << "\t->File name " << green << entry.name << reset << std::endl;
			std::cout << "\t->Entry size " << cyan << sizeof(entry) << reset << std::endl;
			std::cout << "\t->Encrypted length " << cyan << entry.size << reset << std::endl;
		}
		std::string completePath = fileName + config.unpack_pref + entry.name;
		size_t fileNamePos = completePath.find_last_of("/");
		fs::create_directories(completePath.substr(0, fileNamePos));
		if (std::FILE* f1 = std::fopen(completePath.c_str(), "wb")) {

			std::fwrite(&tmp[entry.size], sizeof(unsigned char), entry.fileLength, f1);
			std::fclose(f1);
		}
		else {
			std::cout << "Unknown Error: " << entry.name << std::endl;
			return false;
		}
		++cnt;
	}
	std::cout << "Files unpacked: " << red << cnt << reset << std::endl;
	return true;
}