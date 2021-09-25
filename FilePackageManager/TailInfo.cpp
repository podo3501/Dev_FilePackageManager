#include"stdafx.h"
#include"TailInfo.h"
#include<stack>
#include"File.h"

CTailInfo::CTailInfo(CTailInfo* pTailInfo)
{
	Copy(pTailInfo);
}

CTailInfo::CTailInfo(const std::string& strPackFilename)
	: m_strPackFilename(strPackFilename)
	, m_fileInfo()
{
	m_fileInfo = { {eTailSymbol::DELETE, {} }, {eTailSymbol::MODIFY, {} }, {eTailSymbol::INSERT, {} }, {eTailSymbol::NORMAL, {} } };
}

CTailInfo::~CTailInfo()
{
	m_strPackFilename.clear();
	m_fileInfo.clear();
}

bool CTailInfo::GetPositionList(const std::string& strFilename, PositionList* outPositionList)
{
	for (const auto& fileInfo : m_fileInfo)
	{
		auto fileFind = fileInfo.second.find(strFilename);
		if (fileFind != fileInfo.second.end())
		{
			(*outPositionList) = fileFind->second;
			return true;
		}
	}

	return false;
}

void CTailInfo::GetFileList(eTailSymbol::Type type, FileList* outFileList)
{
	auto find = m_fileInfo.find(type);
	if (find == m_fileInfo.end()) return;

	for (const auto& iter : find->second)
		(*outFileList).emplace_back(iter.first);
}

bool CTailInfo::IsValid()
{
	for (const auto& curFile : m_fileInfo)
		if (curFile.second.empty() != true) return true;

	return false;
}

bool CTailInfo::IsExist(const std::string& strFilename)
{
	for (const auto& curFile : m_fileInfo)
	{
		const FilePositionList& filePositionList = curFile.second;
		auto find = filePositionList.find(strFilename);
		if (find != curFile.second.end())
			return true;
	}

	return false;
}

bool CTailInfo::Copy(CTailInfo* pTailInfo)
{
	if (pTailInfo->IsValid() != true) return false;
	m_strPackFilename.clear();
	m_fileInfo.clear();

	m_strPackFilename = pTailInfo->m_strPackFilename;
	m_fileInfo = pTailInfo->m_fileInfo;

	return true;
}

void MakeStringWithFileList(eTailSymbol::Type type, const CTailInfo::FilePositionList& fileAndPosList, std::string* out)
{
	if (fileAndPosList.empty()) return;

	(*out).append(eTailSymbol::GetString(type) + ",");
	for (const auto& iter : fileAndPosList)
	{
		const std::string& strName = iter.first;
		(*out).append(strName);
		const PositionList& positionList = iter.second;
		for (const auto& currPos : positionList)
		{
			(*out).append("-" + std::to_string(currPos.first));
			(*out).append("-" + std::to_string(currPos.second));
		}
		(*out).append(",");
	}
	(*out).append(eTailSymbol::GetString(eTailSymbol::END) + ",");
}

void CTailInfo::GetData(std::string* out)
{
	(*out).append(eTailSymbol::GetString(eTailSymbol::TAIL) + ",");

	for (const auto& curFile : m_fileInfo)
		MakeStringWithFileList(curFile.first, curFile.second, out);

	(*out).append(eTailSymbol::GetString(eTailSymbol::END));
}

void CTailInfo::ReadFileList(eTailSymbol::Type currType, std::string& strWord)
{
	FileList fileListInfo;
	GetStringListByToken(strWord, '-', &fileListInfo);
	int nIdx = 0;
	std::string fileName;
	PositionList positionList;
	std::pair<__int64, __int64> currPos;
	for (std::string& currStr : fileListInfo)
	{
		if (nIdx == 0)
		{
			fileName = currStr;
			++nIdx;
			continue;
		}
		if (nIdx % 2 == 1)			//홀수순번은 Position 짝수는 Size를 나타낸다.
		{
			currPos.first = std::stoi(currStr);
		}
		else
		{
			currPos.second = std::stoi(currStr);
			positionList.insert(currPos);
			currPos = std::pair<__int64, __int64>(0, 0);
		}
		++nIdx;
	}

	InsertFileInfo(currType, fileName, positionList);
}

void CTailInfo::ReadSymbol(eTailSymbol::Type currType, std::string& strWord)
{
	switch (currType)
	{
	case eTailSymbol::TAIL:		//버전 같은 거나 그런것을 넣는 곳
		break;
	}
}

bool CTailInfo::SetData(const std::string& strTailInfo)
{
	FileList tailWordList;
	GetStringListByToken(strTailInfo, ',', &tailWordList);

	std::stack<eTailSymbol::Type> stackTailSymbol;
	for (std::string& strTailWord : tailWordList)
	{
		eTailSymbol::Type eWord = eTailSymbol::GetType(strTailWord);
		if (eWord != eTailSymbol::NONE)
		{
			if (eWord == eTailSymbol::END)
				stackTailSymbol.pop();
			else
				stackTailSymbol.push(eWord);
			continue;
		}
		eTailSymbol::Type currType = stackTailSymbol.top();
		if (m_fileInfo.find(currType) != m_fileInfo.end())
			ReadFileList(currType, strTailWord);
		else
			ReadSymbol(currType, strTailWord);
	}
	if (stackTailSymbol.empty() == false) return false;

	return true;
}

bool CTailInfo::InsertFileInfo(eTailSymbol::Type type, const std::string& strFilename, const PositionList& positionList)
{
	if (IsExist(strFilename) == true) return false;

	std::pair<std::string, PositionList> curFilePosList(strFilename, positionList);
	auto find = m_fileInfo.find(type);
	if (find == m_fileInfo.end()) return false;

	//find->second.emplace(strFilename, positionList);
	find->second.insert(std::make_pair(strFilename, positionList));

	return true;
}

bool CTailInfo::DeleteFileInfo(eTailSymbol::Type type, const std::string& strFilename)
{
	auto findFilePosList = m_fileInfo.find(type);
	if (findFilePosList == m_fileInfo.end()) return false;

	auto iterFind = findFilePosList->second.find(strFilename);
	if (iterFind == findFilePosList->second.end())
		return false;

	findFilePosList->second.erase(iterFind);

	return true;
}

bool CTailInfo::ConvertDataToTimeLine(TimeLine* outTimeLine)
{
	if (IsValid() != true) return false;

	for (const auto& curFile : m_fileInfo)
	{
		const FilePositionList& filePosList = curFile.second;
		for (const auto& filePos : filePosList)
		{
			const std::string& strFilename = filePos.first;
			const PositionList& positionList = filePos.second;
			for (const auto& curPos : positionList)
			{
				__int64 nTimeLinePos = curPos.first;
				stFilenamePos filenamePos(m_strPackFilename, strFilename, curPos.first, curPos.second);
				const auto& ret = (*outTimeLine).emplace(nTimeLinePos, filenamePos);
				if (ret.second != true) return false;
			}
		}
	}

	if ((*outTimeLine).empty() == true) return false;

	return true;
}

bool CTailInfo::AdjustInfo(const stMovePos& addInfo)
{
	FilePositionList& filePosList = m_fileInfo[eTailSymbol::NORMAL];
	FilePositionList::iterator find = filePosList.find(addInfo.strFilename);
	if (find == filePosList.end())		//insert 되는 파일이다.
	{
		PositionList posList;
		posList.emplace(addInfo.toPos);
		filePosList.emplace(addInfo.strFilename, posList);
		return true;
	}

	PositionList& positionList = find->second;
	if (positionList.empty()) return false;

	auto oriIter = --positionList.end();		//제일 뒤에 있는 블럭이 처음부분이다.
	std::pair<const __int64, __int64>& originalPos = (*oriIter);
	if (originalPos.first == addInfo.fromPos.first && originalPos.second == addInfo.fromPos.second)
		positionList.erase(oriIter);	//전체가 옮겨졌을때는 원본은 필요없다.
	else	//동강이 났을때 원본은 사이즈가 줄어든다.
		originalPos.second -= addInfo.fromPos.second;
	positionList.emplace(addInfo.toPos);

	return true;
}

bool CTailInfo::GetEndPos(const std::string& strFilename, __int64* outEndPos)
{
	FilePositionList& filePosList = m_fileInfo[eTailSymbol::NORMAL];
	FilePositionList::iterator find = filePosList.find(strFilename);
	if (find == filePosList.end()) return false;

	PositionList& positionList = find->second;
	if (positionList.empty()) return false;

	auto& headPos = (*positionList.rbegin());
	(*outEndPos) = headPos.first + headPos.second;

	return true;
}

bool CTailInfo::Write(__int64 nEndFilePart, CFile* pFile)
{
	if (IsValid() == false) return false;

	std::string strTail;
	GetData(&strTail);

	pFile->SetPosition(nEndFilePart);
	pFile->Write(strTail);
	pFile->Write(nEndFilePart);

	return true;
}

bool CTailInfo::Read(CFile* pFile, __int64* outEndFilePart)
{
	__int64 nPos = pFile->GetSize() - sizeof(*outEndFilePart);
	pFile->SetPosition(nPos);
	pFile->Read(outEndFilePart);

	std::string strTailInfo;
	pFile->SetPosition(*outEndFilePart);
	pFile->Read(&strTailInfo);

	return SetData(strTailInfo);
}