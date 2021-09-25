#pragma once
#include"Utility.h"
#include"PackageStruct.h"

class CTailInfo;
class CPatchFile;
class CFile;

class CPatchTimeLine
{
private:
	CPatchTimeLine();

public:
	CPatchTimeLine(CTailInfo* pTailInfo);

	bool MakePatchTimeLine(CPatchFile* pPatchFile);
	void GetToBeMovedList(std::vector<stMovePos>* outToBeMovedList) { (*outToBeMovedList) = m_toBeMovedList; }
	void GetTailInfo(CTailInfo** outTailInfo) { (*outTailInfo) = m_pTailInfo.get(); }
	void GetEndFilePart(__int64* outEndFilePart) { (*outEndFilePart) = m_nEndFilePart; }

private:
	bool MoveDataToEmptySpace(TimeLine& timeLine, PosAndSize emptySpace);
	bool OrganizeSpace(TimeLine& timeLine, const std::vector<PosAndSize>& emptySpace);
	bool DeleteFileFromTimeLine(TimeLine& timeLine, const std::string& strFilename);

private:
	std::unique_ptr<CTailInfo>	m_pTailInfo;
	std::vector<stMovePos>		m_toBeMovedList;
	__int64									m_nEndFilePart;
};

class CPatchFile
{
public:
	CPatchFile();
	~CPatchFile();
	void Close();

	bool CreatePatchFile(const std::string& strTargetPath, const std::string& strBaseDir, const std::string& strFilename, const PatchFileList& patchFileList);
	bool OpenPatchFile(const std::string& strBasePath, const std::string& strPackFilename, eFileType fileOpenType);

	void GetDeleteFileList(FileList* outDeleteFileList);
	bool GetInsertFileList(std::vector<stFilenamePos>* outFileList);
	const std::string& GetPackFilename() { return m_strPackFilename; }

	bool Read(const PosAndSize& pos, stReadData* outReadData);

private:
	void GetInsertFileList(FileList* outInsertFileList);

	bool WriteBody(const PatchFileList& patchFileList);

private:
	std::unique_ptr<CFile>			m_pFile;
	std::string								m_strTargetPath;
	std::string								m_strBasePath;
	std::string								m_strPackFilename;
	std::unique_ptr<CTailInfo>	m_pTailInfo;
	__int64									m_nEndFilePart;
};

