#include"stdafx.h"
#include "PackageFile.h"
#include<io.h>
#include<filesystem>
#include "TailInfo.h"
#include"PatchFile.h"
#include"File.h"

CPackageFile::CPackageFile()
	: m_pFile(nullptr)
	, m_strBasePath()
	, m_strPackFilename()
	, m_pTailInfo(nullptr)
	, m_nEndFilePart(0)
{
}

CPackageFile::~CPackageFile()
{
	Close();
}

void CPackageFile::Close()
{
	m_nEndFilePart = 0;
	m_pTailInfo.reset();
	m_strBasePath.clear();
	m_strPackFilename.clear();
	m_pFile.reset();
}

bool CPackageFile::ReOpen(eFileType fileOpenType)
{
	std::string basePath = m_strBasePath;
	std::string packFilename = m_strPackFilename;
	Close();
	return OpenPackageFile(basePath, packFilename, fileOpenType);
}

bool CPackageFile::IsValid()
{
	if (m_pTailInfo == nullptr) return false;
	if (m_pTailInfo->IsValid() != true) return false;
	if (m_nEndFilePart == 0) return false;
	if (m_pFile == nullptr) return false;
	if (m_pFile->GetSize() == 0) return false;
	if (m_pFile->IsOpen() != true) return false;

	return true;
}

bool CPackageFile::WriteBody(const std::string& strTargetPath, FileList& vecFileList)
{
	for (std::string& strFilename : vecFileList)
	{
		if (m_pTailInfo->IsExist(strFilename)) return false;

		PositionList positionList;
		bool bResult = WriteFile(strTargetPath + strFilename, m_pFile.get(), &positionList);
		if (bResult != true) return false;
		
		m_nEndFilePart = m_pFile->GetPosition();
		m_pTailInfo->InsertFileInfo(eTailSymbol::NORMAL, strFilename, positionList);
	}

	return true;
}

bool CPackageFile::CreatePackageFile(const std::string& strTargetPath, const std::string& strBasePath, const std::string& strPackFilename)
{
	FileList vecFileList;
	GetFileListInDirectory(strTargetPath, &vecFileList);
	if (vecFileList.empty()) return false;
	
	m_pFile = std::make_unique<CFile>(strBasePath + strPackFilename, eFileType::WRITE);
	if (m_pFile->IsOpen() != true) return false;

	m_strBasePath = strBasePath;
	m_strPackFilename = strPackFilename;
	m_pTailInfo = std::make_unique<CTailInfo>(strPackFilename);

	bool bResult = false;
	bResult = WriteBody(strTargetPath, vecFileList);
	if (bResult != true) { Close(); return false; }

	bResult = m_pTailInfo->Write(m_nEndFilePart, m_pFile.get());
	if (bResult != true) { Close(); return false; }

	Close();
	return true;
}

bool CPackageFile::OpenPackageFile(const std::string& strBasePath, const std::string& strPackFilename, eFileType fileOpenType )
{
	m_pFile = std::make_unique<CFile>(strBasePath + strPackFilename, fileOpenType);
	if (m_pFile->IsOpen() != true) return false;

	m_strBasePath = strBasePath;
	m_strPackFilename = strPackFilename;
	m_pTailInfo = std::make_unique<CTailInfo>(strPackFilename);

	bool bResult = m_pTailInfo->Read(m_pFile.get(), &m_nEndFilePart);
	if (bResult != true) { Close(); return false; }

	return true;
}

bool CPackageFile::IsExist(std::string& strFilename)
{
	if (m_pTailInfo == nullptr) return false; 
	if (m_pTailInfo->IsValid() != true) return false;

	return m_pTailInfo->IsExist(strFilename);
}

bool CPackageFile::Read(const std::string& strFilename, stReadData* outReadData)
{
	PositionList curPosList;
	bool bResult = m_pTailInfo->GetPositionList(strFilename, &curPosList);
	if (bResult != true) return false;

	m_pFile->Read(curPosList, outReadData);

	return true;
}

bool CPackageFile::DeleteFileList(CPatchFile* pPatchFile)
{
	FileList deleteFileList;
	pPatchFile->GetDeleteFileList(&deleteFileList);

	for (const auto& strFilename : deleteFileList)
	{
		bool bResult = m_pTailInfo->DeleteFileInfo(eTailSymbol::NORMAL, strFilename);
		if (bResult != true) return false;
	}

	return true;
}

void CPackageFile::CutUnnecessaryPart()
{
	//파일이 삭제되거나 해서 파일의 끝이 안 맞을때 파일의 끝을 잘라준다.
	__int64 nFileEnd = m_pFile->GetPosition();
	std::string strFilename = m_strBasePath + m_strPackFilename;
	m_pFile->Close();
	FILE* pFile = _fsopen(strFilename.c_str(), "r+b", _SH_DENYWR);
	int nFile = _fileno(pFile);
	_chsize_s(nFile, nFileEnd);
	fclose(pFile);
}

//patch
bool CPackageFile::Patch(CPatchFile* pPatchFile)
{
	bool bResult = false;
	std::unique_ptr<CPatchTimeLine> pTimeLine = std::make_unique<CPatchTimeLine>(m_pTailInfo.get());
	bResult = pTimeLine->MakePatchTimeLine(pPatchFile);
	if (bResult != true) return false;

	//READ로 연 것을 MODIFY로 바꾼다.
	bResult = ReOpen(eFileType::MODIFY);
	if (bResult != true) return false;

	std::vector<stMovePos> toBeMovedList;
	pTimeLine->GetToBeMovedList(&toBeMovedList);
	for (const auto& toBeMoved : toBeMovedList)
		m_pFile->Move(toBeMoved, pPatchFile);
	
	CTailInfo* pTailInfo = nullptr; 
	pTimeLine->GetTailInfo(&pTailInfo);
	bResult = m_pTailInfo->Copy(pTailInfo);
	if (bResult != true) return false;

	pTimeLine->GetEndFilePart(&m_nEndFilePart);
	bResult = m_pTailInfo->Write(m_nEndFilePart, m_pFile.get());
	if (bResult != true) return false;

	//패치후에는 닫히는 것이 기본 룰이다.
	CutUnnecessaryPart();
	
	return true;
}

eTailSymbol::Type Compare(CPackageFile* pPackMng, const std::string strBaseDir, const std::string& strFilename)
{
	eTailSymbol::Type type = eTailSymbol::NONE;
	stReadData readData;
	bool bResult = pPackMng->Read(strFilename, &readData);
	//기존의 패키지에 없다면 Insert file이다.
	if(readData.Empty()) 
		return eTailSymbol::INSERT;
	
	std::unique_ptr<CFile> pLoadFile = std::make_unique<CFile>(strBaseDir + strFilename, eFileType::READ);
	bool bEqual = pLoadFile->IsEqual(readData);
	if (bEqual != true)
		return eTailSymbol::MODIFY;

	return type;
}

void CPackageFile::MakePatchFileList(const std::string& strPatchDir, PatchFileList* outList)
{
	FileList packFileList;
	m_pTailInfo->GetFileList(eTailSymbol::NORMAL, &packFileList);

	for (const auto& file : std::filesystem::recursive_directory_iterator(strPatchDir.c_str()))
	{
		if (file.is_directory()) continue;

		const std::string& strPullPath = file.path().string();
		std::string strFilename;
		DividePathAndName(strPullPath, strPatchDir, &strFilename);

		eTailSymbol::Type type = Compare(this, strPatchDir, strFilename);
		switch (type)
		{
		case eTailSymbol::MODIFY:
		case eTailSymbol::INSERT:
			(*outList)[type].emplace_back(strFilename);
			break;
		}
		//팩에는 있고 현재폴더에 없다면 지워지는 파일들이다.
		auto iterFind = std::find(packFileList.begin(), packFileList.end(), strFilename);
		if (iterFind != packFileList.end()) packFileList.erase(iterFind);
	}
	//팩 파일 리스트에서 남은 파일은 지워야 하는 파일이다.
	(*outList)[eTailSymbol::DELETE] = packFileList;
}

bool CPackageFile::MakePatchFile(const std::string& strPatchDir, const std::string& strBaseDir, const std::string& strPatchName, std::unique_ptr<CPatchFile>* outPatch)
{
	if (IsValid() != true) return false;

	PatchFileList patchFileList = { {eTailSymbol::DELETE, {} }, {eTailSymbol::MODIFY, {} }, {eTailSymbol::INSERT, {} } };
	MakePatchFileList(strPatchDir, &patchFileList);

	//패치할 목록으로 패치파일을 만든다.
	(*outPatch) = std::make_unique<CPatchFile>();
	bool bResult = (*outPatch)->CreatePatchFile (strPatchDir, strBaseDir, strPatchName, patchFileList);
	if (bResult != true) return bResult;

	return true;
}
