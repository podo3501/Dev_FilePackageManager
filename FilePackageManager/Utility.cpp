#include"stdafx.h"
#include"Utility.h"
#include<sstream>
#include<filesystem>
#include"File.h"

void MemoryLeakCheck()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	_CrtSetBreakAlloc(498);
}

void GetStringListByToken(const std::string& str, const char separate, std::vector<std::string>* outStrList)
{
	std::stringstream ssTailInfo(str);
	std::string token;
	while (std::getline(ssTailInfo, token, separate))
		(*outStrList).emplace_back(token);
}

bool DividePathAndName(const std::string& strPullPath, const std::string& strBasePath, std::string* outFilename)
{
	size_t nFind = strPullPath.find(strBasePath);

	if (nFind == std::string::npos) return false;

	size_t nPos = strBasePath.size();
	(*outFilename) = strPullPath.substr(nPos, strPullPath.length() - nPos);

	return true;
}

void GetFileListInDirectory(const std::string& strTargetPath, FileList* outFileList)
{
	for (const auto& file : std::filesystem::recursive_directory_iterator(strTargetPath))
	{
		if (file.is_directory()) continue;
		std::string strFilename;
		DividePathAndName(file.path().string(), strTargetPath, &strFilename);
		(*outFileList).emplace_back(strFilename);
	}
}

bool WriteFile(const std::string& strFilename, CFile* pFile, PositionList* outPosList)
{
	std::unique_ptr<CFile> loadFile(std::make_unique<CFile>(strFilename, eFileType::READ));
	if (loadFile->IsOpen() != true) return false;

	__int64 nBegPos = pFile->GetPosition();
	stReadData readData;
	loadFile->Read(0, &readData);
	pFile->Write(readData);

	(*outPosList).emplace(nBegPos, readData.nSize);

	return true;
}


