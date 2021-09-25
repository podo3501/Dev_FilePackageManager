#pragma once
#include<vector>
#include<map>
#include<string>
#include<memory>

typedef std::pair<__int64, __int64> PosAndSize;
typedef std::map<__int64, __int64> PositionList;

inline __int64 GetEndPos(const PosAndSize& posAndSize)
{
	return posAndSize.first + posAndSize.second;
}

struct stFilenamePos
{
public:
	stFilenamePos()
		: strPackname()
		, strFilename()
		, fromPos()
	{}

	stFilenamePos(const std::string& _packName, const std::string& _filename, const PosAndSize& _from)
		: strPackname(_packName)
		, strFilename(_filename)
		, fromPos(_from)
	{}

	stFilenamePos(const std::string& _packName, const std::string& _filename, const __int64& _nPos, const __int64& _nSize)
		: strPackname(_packName)
		, strFilename(_filename)
		, fromPos(_nPos, _nSize)
	{}

	void Set(const std::string& _packName, const std::string& _filename, const PosAndSize& _from)
	{
		strPackname = _packName;
		strFilename = _filename;
		fromPos = _from;
	}

	bool IsEqual(const stFilenamePos& rhs)
	{
		if (strPackname != rhs.strPackname) return false;
		if (strFilename != rhs.strFilename) return false;
		if (fromPos != rhs.fromPos) return false;

		return true;
	}

public:
	std::string		strPackname;
	std::string		strFilename;
	PosAndSize	fromPos;
};

struct stMovePos : public stFilenamePos
{
public:
	stMovePos()
		: stFilenamePos()
		, toPos()
	{}

	stMovePos(const stFilenamePos& _filenamePos, const PosAndSize& _to)
		: stFilenamePos(_filenamePos)
		, toPos(_to)
	{}

	stMovePos(const std::string& _packName, const std::string& _filename, const PosAndSize& _from, const PosAndSize& _to)
		: stFilenamePos(_packName, _filename, _from)
		, toPos(_to)
	{}

	void Set(const std::string& _packName, const std::string& _filename, const PosAndSize& _from, const PosAndSize& _to)
	{
		stFilenamePos::Set(_packName, _filename, _from);
		toPos = _to;
	}

	bool IsEqual(const stMovePos& rhs)
	{
		if (stFilenamePos::IsEqual(rhs) == false) return false;
		if (toPos != rhs.toPos) return false;

		return true;
	}

public:
	PosAndSize	toPos;
};

namespace eTailSymbol
{
	enum Type
	{
		NONE = 0,
		TAIL,
		DELETE,
		MODIFY,
		INSERT,
		NORMAL,
		END
	};

	std::string GetString(Type eWord);
	Type GetType(std::string& str);
}

typedef __int64 TimeLinePos;
typedef std::map<TimeLinePos, stFilenamePos, std::greater<TimeLinePos>> TimeLine;

typedef std::vector<std::string> FileList;
typedef std::map<eTailSymbol::Type, FileList> PatchFileList;


enum eFileType
{
	NONE = 0,
	WRITE = 1,
	READ = 2,
	MODIFY = 3,
};

struct stReadData
{
public:
	stReadData()
		: pData(nullptr),
		nSize(0)
	{}

	bool Empty()
	{
		if (pData != nullptr) return false;
		if (nSize != 0) return false;

		return true;
	}

public:
	std::unique_ptr<unsigned char[]> pData;
	__int64 nSize;
};