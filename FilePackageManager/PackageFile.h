#pragma once
#include"PackageStruct.h"
#include"Utility.h"

class CFile;
class CTailInfo;
class CPatchFile;

class CPackageFile
{
public:
	CPackageFile();
	~CPackageFile();

	void Close();
	bool IsValid();

	bool CreatePackageFile(const std::string& strTargetPath, const std::string& strBasePath, const std::string& strPackFilename);
	bool OpenPackageFile(const std::string& strBasePath, const std::string& strPackFilename, eFileType fileOpenType);

	bool IsExist(std::string& strFilename);
	
	bool Read(const std::string& strFilename, stReadData* outReadData);

private:
	bool ReOpen(eFileType fileOpenType);

	bool WriteBody(const std::string& strTargetPath, FileList& vecFileList);

	//patch
public:
	bool MakePatchFile(const std::string& strPatchDir, const std::string& strBaseDir, const std::string& strPatchName, std::unique_ptr<CPatchFile>* outPatch);
	bool Patch(CPatchFile* pPatchFile);

private:
	void CutUnnecessaryPart();
	void MakePatchFileList(const std::string& strPatchDir, PatchFileList* outList);
	bool DeleteFileList(CPatchFile* pPatchFile);

private:
	std::unique_ptr<CFile>			m_pFile;
	std::string								m_strBasePath;
	std::string								m_strPackFilename;
	std::unique_ptr<CTailInfo>	m_pTailInfo;
	__int64									m_nEndFilePart;
};