#pragma once
#include"PackageStruct.h"
#include<fstream>

class CPatchFile;

class CFile
{
public:
	CFile() = delete;
	CFile(const std::string& strFilename, eFileType type);
	~CFile();

	bool Move(const stMovePos& movePos, CPatchFile* pPatchFile);

	bool Read(const PosAndSize& pos, stReadData* outReadData);
	bool Read(__int64 nPos, __int64 nSize, stReadData* outReadData);
	bool Read(__int64 nSize, stReadData* outReadData);
	bool Read(const std::map<__int64, __int64>& curPosList, stReadData* outReadData);

	bool IsEqual(CFile* rhs);
	bool IsEqual(stReadData& readData);

	void Close();
	bool IsOpen();

	__int64 GetPosition();
	__int64 GetSize();

	void SetPosition(__int64 nPos);

private:
	eFileType		m_type;
	std::basic_fstream<unsigned char> m_sFile;

//템플렛 헤드에서 분리하는 방법을 강구해 보자.
public:
	template<typename T>
	void Read(T* out)					//int같은 숫자
	{
		m_sFile.read(reinterpret_cast<unsigned char*>(out), sizeof(T));
	}

	template<>
	void Read(std::string* out)
	{
		unsigned char* cBuffer = nullptr;
		int nSize = 0;
		Read(&nSize);
		cBuffer = new unsigned char[nSize +1];
		cBuffer[nSize] = '\0';
		m_sFile.read(&cBuffer[0], nSize);
		(*out) = static_cast<std::string>(reinterpret_cast<const char*>(cBuffer));
		delete[] cBuffer;
	}


	//template<typename T>
	//void ReadFromPos(__int64 nPos, T* out)
	//{
	//	m_sFile.seekg(nPos, std::ios::beg);
	//	Read(out);
	//}

public:
	//Write
	void Write(const PosAndSize& pos, const stReadData& readData)
	{
		SetPosition(pos.first);
		Write(readData);
	}

	void Write(const stReadData& readData)
	{
		m_sFile.write(readData.pData.get(), readData.nSize);
	}

	template<typename T>
	void Write(T nIn)									//size_t, int, __int64
	{
		m_sFile.write(reinterpret_cast<unsigned char*>(&nIn), sizeof(T));
	}
	template<>
	void Write(std::string nIn)
	{
		std::basic_string<unsigned char> uc_str(reinterpret_cast<const unsigned char*>(nIn.c_str()));
		int nSize = uc_str.size();
		Write(nSize);
		m_sFile.write(uc_str.c_str(), nSize);
	}
	//Write Size
	template<typename T>
	void Write(T nIn, __int64 nSize)			//unsigned char
	{
		m_sFile.write(nIn, nSize);
	}
	template<typename T>
	void WriteFromPos(__int64 nPos,T nIn)
	{
		m_sFile.seekg(nPos, std::ios::beg);
		Write(nIn);
	}
};

