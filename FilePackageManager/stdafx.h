#ifndef PACKAGE_PCH_H
#define PACKAGE_PCH_H

// ���⿡ �̸� �������Ϸ��� ��� �߰�
#include<string>

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif //PACKAGE_PCH_H