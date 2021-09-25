#include"stdafx.h"
#include"PatchFile.h"
#include"PackageFile.h"
#include"TailInfo.h"
#include"File.h"

CPatchTimeLine::CPatchTimeLine(CTailInfo* pTailInfo)
	: m_pTailInfo(nullptr)
	, m_toBeMovedList()
	, m_nEndFilePart(0)
{
	m_pTailInfo = std::make_unique<CTailInfo>(pTailInfo);
}

bool CPatchTimeLine::DeleteFileFromTimeLine(TimeLine& timeLine, const std::string& strFilename)
{
	bool bResult = false;
	for (auto iter = timeLine.begin(); iter != timeLine.end();)
	{
		stFilenamePos& filenamePos = iter->second;
		if (filenamePos.strFilename != strFilename)
		{
			++iter;
			continue;
		}

		timeLine.erase(iter++);
		bResult = true;
	}

	return bResult;
}

void ReduceSpace(__int64 nSize, PosAndSize* outEmptySpace)
{
	(*outEmptySpace).first += nSize;
	(*outEmptySpace).second -= nSize;
}

bool CPatchTimeLine::MoveDataToEmptySpace(TimeLine& timeLine, PosAndSize emptySpace)
{
	_int64 nEndFilePart = GetEndPos(timeLine.begin()->second.fromPos);
	if (nEndFilePart <= emptySpace.first) return true;		//앞에서 큰 파일이 빠지면 연속해서 뒤에 빈 공간이 생길때 문제가 생겼다.

	for (TimeLine::iterator iter = timeLine.begin(); iter != timeLine.end();)
	{
		bool bIter = false;
		TimeLinePos timeLinePos = iter->first;
		stFilenamePos& curFilenamePos = iter->second;
		PosAndSize& curPos = curFilenamePos.fromPos;
		__int64 nCurStartPos = iter->first;
		__int64 nCurEndPos = GetEndPos(curFilenamePos.fromPos);
		__int64 nCurSpaceEndPos = GetEndPos(emptySpace);
		__int64 nCutPos = nCurEndPos - emptySpace.second;

		if (nCutPos <= nCurStartPos || nCurSpaceEndPos == nCurStartPos) //전체를 옮긴다.
		{
			__int64 nSize = curPos.second;
			stMovePos moveCut;
			PosAndSize toPos(emptySpace.first, nSize);
			moveCut.Set(curFilenamePos.strPackname, curFilenamePos.strFilename,
				curFilenamePos.fromPos, toPos);
			m_toBeMovedList.emplace_back(moveCut);
			ReduceSpace(nSize, &emptySpace);

			stFilenamePos movePos(curFilenamePos.strPackname, curFilenamePos.strFilename, toPos);
			auto deleteIter = timeLine.find(timeLinePos);
			timeLine.erase(iter++); bIter = true;
			timeLine.emplace(toPos.first, movePos);
		}
		else if (nCurStartPos < nCutPos && nCutPos < nCurEndPos) //파일이 동강이 난다.
		{
			__int64 nSize = emptySpace.second;
			stMovePos moveCut;
			PosAndSize toPos(emptySpace.first, nSize);
			moveCut.Set(curFilenamePos.strPackname, curFilenamePos.strFilename,
				PosAndSize(nCutPos, nSize), toPos);
			m_toBeMovedList.emplace_back(moveCut);
			ReduceSpace(nSize, &emptySpace);

			curPos.second -= nSize;	//잘려진 앞 부분은 사이즈를 줄이고 자른 부분은 새로 넣어준다.
			stFilenamePos movePos(curFilenamePos.strPackname, curFilenamePos.strFilename, toPos);
			timeLine.emplace(toPos.first, movePos);
		}
		else
			break;
		const PosAndSize& endFile = timeLine.begin()->second.fromPos;
		if (emptySpace.first >= GetEndPos(endFile)) break;		//파일 끝에 남은 공간은 필요없다.
		if (emptySpace.second <= 0) break;								//빈공간이 없으면
		if(bIter == false) iter++;
	}

	return true;
}

bool CPatchTimeLine::OrganizeSpace(TimeLine& timeLine, const std::vector<PosAndSize>& emptySpace)
{
	if (timeLine.empty()) return true;

	for (const auto& curSpace : emptySpace)
	{
		bool bResult = MoveDataToEmptySpace(timeLine, curSpace);
		if (bResult != true) return false;
	}

	m_nEndFilePart = GetEndPos(timeLine.begin()->second.fromPos);

	return true;
}

bool CPatchTimeLine::MakePatchTimeLine(CPatchFile* pPatchFile)
{
	//지울 파일들과 넣을 파일들로 타임라인을 만든다.
	//실제로 패치를 하기 전에 정보를 가지고 임의로 어떻게 패치할지 리스트를 만든다.
	TimeLine timeLine;
	bool bResult = m_pTailInfo->ConvertDataToTimeLine(&timeLine);
	if (bResult != true) return false;
	//지우고
	FileList deleteFileList;
	pPatchFile->GetDeleteFileList(&deleteFileList);
	for (const std::string& strFilename : deleteFileList)
	{
		bResult = DeleteFileFromTimeLine(timeLine, strFilename);
		if (bResult != true) return false;
	}
	//빈공간 확인
	std::vector<PosAndSize> emptySpace;
	__int64 nStartPos = 0;
	for (auto rit = timeLine.rbegin(); rit != timeLine.rend(); rit++)
	{
		__int64 nFilePos = rit->first;
		if (nStartPos != nFilePos)
			emptySpace.emplace_back(nStartPos, nFilePos - nStartPos);
		nStartPos = GetEndPos(rit->second.fromPos);
	}
	
	OrganizeSpace(timeLine, emptySpace);

	//새로운 파일 패치
	std::vector<stFilenamePos> insertFileList;
	pPatchFile->GetInsertFileList(&insertFileList);
	for (const auto& filenamePos : insertFileList)
	{
		PosAndSize toPos(m_nEndFilePart, filenamePos.fromPos.second);
		stMovePos movePos(filenamePos, toPos);
		m_toBeMovedList.emplace_back(movePos);

		stFilenamePos timeLinePos(filenamePos.strPackname, filenamePos.strFilename, toPos);
		timeLine.emplace(m_nEndFilePart, timeLinePos);
		m_nEndFilePart = GetEndPos(timeLine.begin()->second.fromPos);
	}

	//Tail 정리
	for (const std::string& strFilename : deleteFileList)
	{
		bool bResult = m_pTailInfo->DeleteFileInfo(eTailSymbol::NORMAL, strFilename);
		if (bResult != true) return false;
	}
	for (const auto& toBeMoved : m_toBeMovedList)
	{
		bool bResult = m_pTailInfo->AdjustInfo(toBeMoved);
		if (bResult != true) return false;
	}
	

	return true;
}

/// <summary>
/// 
/// </summary>

CPatchFile::CPatchFile()
	: m_pFile(nullptr)
	, m_strTargetPath()
	, m_strBasePath()
	, m_strPackFilename()
	, m_pTailInfo(nullptr)
	, m_nEndFilePart(0)
{
}

CPatchFile::~CPatchFile()
{
	Close();
}

void CPatchFile::Close()
{
	m_nEndFilePart = 0;
	m_pTailInfo.reset();
	m_strPackFilename.clear();
	m_strBasePath.clear();
	m_strTargetPath.clear();
	m_pFile.reset();
}

bool CPatchFile::WriteBody(const PatchFileList& patchFileList)
{
	bool bResult = false;

	for (const auto& file : patchFileList)
	{
		eTailSymbol::Type curType = file.first;
		const FileList& curFileList = file.second;

		for (const auto& filename : curFileList)
		{
			if (m_pTailInfo->IsExist(filename)) return false;

			PositionList positionList;
			if (curType != eTailSymbol::DELETE)	//지워질 파일은 저장하지 않는다.
			{
				bool bResult = WriteFile(m_strTargetPath + filename, m_pFile.get(), &positionList);
				if (bResult != true) return false;
				m_nEndFilePart = m_pFile->GetPosition();
			}
			m_pTailInfo->InsertFileInfo(curType, filename, positionList);
		}
	}

	return true;
}

bool IsEmpty(const PatchFileList& patchFileList)
{
	for (const auto& fileList : patchFileList)
		if (fileList.second.empty() != true) return false;

	return true;
}

bool CPatchFile::CreatePatchFile(const std::string& strTargetPath, const std::string& strBaseDir, const std::string& strFilename, const PatchFileList& patchFileList)
{
	if (IsEmpty(patchFileList)) return false;

	m_pFile = std::make_unique<CFile>(strBaseDir + strFilename, eFileType::WRITE);
	if (m_pFile->IsOpen() != true) return false;

	m_strTargetPath = strTargetPath;
	m_strBasePath = strBaseDir;
	m_strPackFilename = strFilename;
	m_pTailInfo = std::make_unique<CTailInfo>(strFilename);

	bool bResult = WriteBody(patchFileList);
	if (bResult != true) { Close(); return false; }

	bResult = m_pTailInfo->Write(m_nEndFilePart, m_pFile.get());
	if (bResult != true) { Close(); return false; }

	Close();
	return true;
}

bool CPatchFile::OpenPatchFile(const std::string& strBasePath, const std::string& strPackFilename, eFileType fileOpenType)
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

void GetFileList(CTailInfo* pTailInfo, eTailSymbol::Type type, FileList* outDeleteFileList)
{
	FileList fileList;
	pTailInfo->GetFileList(type, &fileList);
	(*outDeleteFileList).insert((*outDeleteFileList).end(), fileList.begin(), fileList.end());
}

void CPatchFile::GetDeleteFileList(FileList* outDeleteFileList)
{
	(*outDeleteFileList).clear();
	GetFileList(m_pTailInfo.get(), eTailSymbol::DELETE, outDeleteFileList);
	GetFileList(m_pTailInfo.get(), eTailSymbol::MODIFY, outDeleteFileList);
}

void CPatchFile::GetInsertFileList(FileList* outInsertFileList)
{
	(*outInsertFileList).clear();
	GetFileList(m_pTailInfo.get(), eTailSymbol::MODIFY, outInsertFileList);
	GetFileList(m_pTailInfo.get(), eTailSymbol::INSERT, outInsertFileList);
}

bool CPatchFile::GetInsertFileList(std::vector<stFilenamePos>* outFileList)
{
	const std::string& strPackname = m_pTailInfo->GetPackFilename();
	FileList fileList;
	GetInsertFileList(&fileList);
	for (const std::string& strFilename : fileList)
	{
		PositionList posList;
		bool bResult = m_pTailInfo->GetPositionList(strFilename, &posList);
		if (bResult != true) return false;
		if (posList.size() > 1) return false;

		stFilenamePos filenamePos;
		filenamePos.strPackname = strPackname;
		filenamePos.strFilename = strFilename;
		filenamePos.Set(strPackname, strFilename, (*posList.begin()));
		(*outFileList).emplace_back(filenamePos);
	}

	return true;
}

bool CPatchFile::Read(const PosAndSize& pos, stReadData* outReadData)
{
	return m_pFile->Read(pos, outReadData);
}