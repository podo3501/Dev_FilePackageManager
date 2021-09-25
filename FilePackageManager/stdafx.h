#ifndef PACKAGE_PCH_H
#define PACKAGE_PCH_H

// 여기에 미리 컴파일하려는 헤더 추가
#include<string>

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif //PACKAGE_PCH_H