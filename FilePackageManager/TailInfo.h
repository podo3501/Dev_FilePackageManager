#pragma once
#include<unordered_map>
#include"PackageStruct.h"
#include"Utility.h"

class CFile;

class CTailInfo
{
public:
	typedef std::unordered_map<std::string, PositionList>	FilePositionList;
	typedef std::map<eTailSymbol::Type, FilePositionList>	FileInfo;

private:
	CTailInfo();

public:
	CTailInfo(CTailInfo* pTailInfo);
	CTailInfo(const std::string& strPackFilename);
	~CTailInfo();

	bool GetPositionList(const std::string& strFilename, PositionList* outPositionList);
	void GetFileList(eTailSymbol::Type type, FileList* outFileList);
	const std::string& GetPackFilename() { return m_strPackFilename; }

	bool IsValid();
	bool IsExist(const std::string& strFilename);
	bool Copy(CTailInfo* pTailInfo);

	bool InsertFileInfo(eTailSymbol::Type type, const std::string& strFilename, const PositionList& positionList);
	bool DeleteFileInfo(eTailSymbol::Type type, const std::string& strFilename);

	bool ConvertDataToTimeLine(TimeLine* outTimeLine);
	bool AdjustInfo(const stMovePos& addInfo);
	bool GetEndPos(const std::string& strFilename, __int64* outEndPos);

	bool Write(__int64 nEndFilePart, CFile* pFile);
	bool Read(CFile* pFile, __int64* outEndFilePart);

private:
	void GetData(std::string* out);
	bool SetData(const std::string& strTailInfo);

	void ReadFileList(eTailSymbol::Type currType, std::string& strWord);
	void ReadSymbol(eTailSymbol::Type currType, std::string& strWord);

private:
	std::string			m_strPackFilename;
	FileInfo				m_fileInfo;
};
