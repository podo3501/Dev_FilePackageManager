#include"stdafx.h"
#include "File.h"
#include "PatchFile.h"

void OpenFile(const std::string& strFilename, eFileType type, std::basic_fstream<unsigned char>& sFile)
{
    int nRW = 0;
    switch (type)
    {
    case eFileType::WRITE:        nRW = std::ios::out | std::ios::trunc;    break;
    case eFileType::READ:         nRW = std::ios::in;    break;
    case eFileType::MODIFY:    nRW = std::ios::in | std::ios::out; break;
    }

    sFile.open(strFilename, nRW | std::ios::binary);
}

CFile::CFile(const std::string& strFilename, eFileType type)
	: m_type(type)
    , m_sFile()
{
    OpenFile(strFilename, m_type, m_sFile);
}

CFile::~CFile()
{
    if (m_sFile.is_open())
        m_sFile.close();
}

void CFile::Close()
{
    if (m_sFile.is_open())
        m_sFile.close();
}

bool CFile::IsEqual(CFile* rhs)
{
    stReadData rhsData;
    rhs->Read(0, &rhsData);

    return IsEqual(rhsData);
}

bool CFile::IsEqual(stReadData& readData)
{
    stReadData lhsData;
    Read(0, &lhsData);

    if (lhsData.nSize != readData.nSize)  return false;
    if (memcmp(lhsData.pData.get(), readData.pData.get(), static_cast<size_t>(readData.nSize)) != 0)
        return false;

    return true;
}

bool CFile::IsOpen()
{
    return m_sFile.is_open();
}

bool CFile::Move(const stMovePos& movePos, CPatchFile* pPatchFile)
{
    bool bResult = false;
    if (movePos.toPos == PosAndSize(0, 0)) return true;  //마지막 파일이 delete될때는 다른 마지막 파일이 없기 때문에 이동할 곳이 없다.

    stReadData readData;
    if (pPatchFile->GetPackFilename() == movePos.strPackname)
        bResult = pPatchFile->Read(movePos.fromPos, &readData);
    else
        bResult = Read(movePos.fromPos, &readData); //패키지 안에 파일이라면 이동을 한다.
        
    if (bResult != true) return false;
    Write(movePos.toPos, readData);

    return true;
}

bool CFile::Read(const PosAndSize& pos, stReadData* outReadData)
{
    SetPosition(pos.first);
    return Read(pos.second, outReadData);
}

bool CFile::Read(__int64 nPos, __int64 nSize, stReadData* outReadData)
{
    SetPosition(nPos);
    return Read(nSize, outReadData); 
}

bool CFile::Read(__int64 nSize, stReadData* outReadData)
{
    if (m_sFile.is_open() != true) 
        return false;

    if( nSize == 0)
        nSize = GetSize();

    std::unique_ptr<unsigned char[]> pBuffer = std::make_unique<unsigned char[]>(static_cast<size_t>(nSize));
    m_sFile.read(&pBuffer[0], nSize);
    (*outReadData).pData = std::move(pBuffer);
    (*outReadData).nSize = nSize;

	return true;
}

bool CFile::Read(const std::map<__int64, __int64>& positionList, stReadData* outReadData)
{
    __int64 nTotalSize = 0;
    for (const auto& curPos : positionList)
        nTotalSize += curPos.second;

    std::unique_ptr<unsigned char[]> pBuffer = std::make_unique<unsigned char[]>(static_cast<size_t>(nTotalSize));
    size_t nBufferPos = 0;
    
    for(auto ritCurPos = positionList.rbegin(); ritCurPos != positionList.rend(); ++ritCurPos)
    {   //배열 크기가 unsigned int를 넘길 수 없다.  즉 개별파일 4기가 정도이다. 넘어가면 어쩌지?
        const size_t& nCurSize = static_cast<size_t>(ritCurPos->second);
        SetPosition(ritCurPos->first);
        m_sFile.read(&pBuffer[nBufferPos], nCurSize);
        nBufferPos += ( nCurSize );  
    }

    (*outReadData).pData = std::move(pBuffer);
    (*outReadData).nSize = nTotalSize;

    return true;
}

__int64 CFile::GetPosition()
{
    return static_cast<__int64>(m_sFile.tellg());
}

void CFile::SetPosition(__int64 nPos)
{
    m_sFile.seekg(nPos, std::ios::beg);
}

__int64 CFile::GetSize()
{
    m_sFile.seekg(0, std::ios::end);
    __int64 nSize = static_cast<__int64>(m_sFile.tellg());
    m_sFile.seekg(0, std::ios::beg);

    return nSize;
}

