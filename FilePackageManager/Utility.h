#pragma once
#include<string>
#include<vector>
#include"PackageStruct.h"

class CFile;

void MemoryLeakCheck();
void GetStringListByToken(const std::string& str, const char separate, std::vector<std::string>* outStrList);
bool DividePathAndName(const std::string& strPullPath, const std::string& strBasePath, std::string* outFilename);
void GetFileListInDirectory(const std::string& strTargetPath, FileList* outFileList);
bool WriteFile(const std::string& strFilename, CFile* pFile, PositionList* outPosList);