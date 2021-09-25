#include"stdafx.h"
#include"PackageStruct.h"

std::string eTailSymbol::GetString(Type eWord)
{
	if (eWord == TAIL) return "Tail";
	if (eWord == DELETE) return "Delete";
	if (eWord == MODIFY) return "Modify";
	if (eWord == INSERT) return "Insert";
	if (eWord == NORMAL) return "Normal";
	if (eWord == END) return "End";

	return "";
}

eTailSymbol::Type eTailSymbol::GetType(std::string& str)
{
	if (str == "Tail") return TAIL;
	if (str == "Delete") return DELETE;
	if (str == "Modify") return MODIFY;
	if (str == "Insert") return INSERT;
	if (str == "Normal") return NORMAL;
	if (str == "End") return END;

	return NONE;
}