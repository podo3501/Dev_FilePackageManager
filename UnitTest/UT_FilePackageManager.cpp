#include "pch.h"
#include "CppUnitTest.h"
#include<filesystem>
#include<random>
#include <iostream>
#include <chrono>
#include <thread>
#include "../FilePackageManager/PackageFile.h"
#include "../FilePackageManager/PatchFile.h"
#include "../FilePackageManager/File.h"
#include "../FilePackageManager/Utility.h"

//#define _CRTDBG_MAP_ALLOC
//#include <cstdlib>
#include <crtdbg.h>
//
//#ifdef _DEBUG
//#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const std::string STR_CUR_TEST_DIR = STR_TEST_RESOURCE +"TestFilePackageManager\\"; 
const std::string strPackName = "Package\\pack.pak";
const std::string strPatchName = "Patch\\patch.pak";
const std::string STR_VERSION_1_DIR = STR_CUR_TEST_DIR + "Version1\\";
const std::string STR_VERSION_2_DIR = STR_CUR_TEST_DIR + "Version2\\";

class CTest
{
	std::string strName;
	std::unique_ptr<CFile> m_pFile;
};

namespace UT_FilePackageManager
{
	TEST_CLASS(UT_FilePackageManager)
	{
	private:
		std::unique_ptr<CPackageFile> m_pPackageFile;
		FileList m_ver1FileList, m_ver2FileList;

	public:
		UT_FilePackageManager()
			: m_pPackageFile(),
			m_ver1FileList(),
			m_ver2FileList()
		{}

		TEST_METHOD_INITIALIZE(MethodInitialize)
		{
#if defined(DEBUG) | defined(_DEBUG)
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);// | _CRTDBG_CHECK_ALWAYS_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif
			m_pPackageFile = std::make_unique<CPackageFile>();
			bool bResult = m_pPackageFile->CreatePackageFile(STR_VERSION_1_DIR, STR_CUR_TEST_DIR, strPackName);
			m_pPackageFile->Close();
			Assert::AreEqual(true, bResult);

			GetFileListInDirectory(STR_VERSION_1_DIR, &m_ver1FileList);
			GetFileListInDirectory(STR_VERSION_2_DIR, &m_ver2FileList);
		}

		TEST_METHOD_CLEANUP(MethodCleanUp)
		{
		}
		
	public:
		//헤드파일 읽어오기
		TEST_METHOD(TestGetTail)
		{
			bool bResult = false;
			bResult = m_pPackageFile->OpenPackageFile(STR_CUR_TEST_DIR, strPackName, eFileType::READ);
			Assert::AreEqual(true, bResult);
			
			for (std::string& currFilename : m_ver1FileList)
			{
				bResult = m_pPackageFile->IsExist(currFilename);
				Assert::AreEqual(true, bResult);
			}
		}

		bool DiffFileList(CPackageFile* pPackageFile, const FileList& fileList, const std::string& strDir)
		{
			for (const std::string& strFilename : fileList)
			{
				stReadData readData;
				bool bResult = pPackageFile->Read(strFilename, &readData);
				if (bResult != true) return false;

				std::unique_ptr<CFile> pLoadFile = std::make_unique<CFile>(strDir + strFilename, eFileType::READ);
				bResult = pLoadFile->IsEqual(readData);
				if (bResult != true) return false;
			}
			return true;
		}
		//파일 읽어오기
		TEST_METHOD(TestReadFile)
		{
			m_pPackageFile->OpenPackageFile(STR_CUR_TEST_DIR, strPackName, eFileType::READ);
			bool bResult = m_pPackageFile->IsValid();
			Assert::AreEqual(true, bResult);

			//파일비교
			bResult = DiffFileList(m_pPackageFile.get(), m_ver1FileList, STR_VERSION_1_DIR);
			Assert::AreEqual(true, bResult);
		}

		bool SameData(const std::string& strDir_l, const std::string& strDir_r)
		{
			FileList fileList_l, fileList_r;
			GetFileListInDirectory(strDir_l, &fileList_l);
			GetFileListInDirectory(strDir_r, &fileList_r);

			for (const std::string& strFilename : fileList_r)
			{
				auto find = std::find(fileList_l.begin(), fileList_l.end(), strFilename); 
				if (find == fileList_l.end()) return false;
			}

			for (const std::string& strFilename : fileList_r)
			{
				std::unique_ptr<CFile> pFile_r = std::make_unique<CFile>(STR_VERSION_2_DIR + strFilename, eFileType::READ);
				std::unique_ptr<CFile> pFile_l = std::make_unique<CFile>(STR_VERSION_1_DIR + strFilename, eFileType::READ);

				if (pFile_r->IsEqual(pFile_l.get()) == false)
					return false;
			}
			return true;
		}

		TEST_METHOD(TestPatch)
		{
			std::unique_ptr<CPackageFile> pPackageFile = std::make_unique<CPackageFile>();
			bool bResult = pPackageFile->CreatePackageFile(STR_VERSION_1_DIR, STR_CUR_TEST_DIR, strPackName);
			m_pPackageFile->Close();
			Assert::AreEqual(true, bResult);

			bResult = pPackageFile->OpenPackageFile(STR_CUR_TEST_DIR, strPackName, eFileType::READ);
			Assert::AreEqual(true, bResult);

			std::unique_ptr<CPatchFile> pPatchFile;
			bResult = pPackageFile->MakePatchFile(STR_VERSION_2_DIR, STR_CUR_TEST_DIR, strPatchName, &pPatchFile);
			if (bResult != true)
			{
				bool bSame = SameData(STR_VERSION_1_DIR, STR_VERSION_2_DIR);
				if (bSame == true)
					return;
			}
			Assert::AreEqual(true, bResult);
			pPatchFile.reset();

			pPatchFile = std::make_unique<CPatchFile>();
			bResult = pPatchFile->OpenPatchFile(STR_CUR_TEST_DIR, strPatchName, eFileType::READ);
			Assert::AreEqual(true, bResult);

			bResult = pPackageFile->Patch(pPatchFile.get());
			Assert::AreEqual(true, bResult);
			pPatchFile.reset();
			pPackageFile.reset();
			
			pPackageFile = std::make_unique<CPackageFile>();
			bResult = pPackageFile->OpenPackageFile(STR_CUR_TEST_DIR, strPackName, eFileType::READ);
			Assert::AreEqual(true, bResult);

			FileList ver2FileList;
			GetFileListInDirectory(STR_VERSION_2_DIR, &ver2FileList);
			bResult = DiffFileList(pPackageFile.get(), ver2FileList, STR_VERSION_2_DIR);
			Assert::AreEqual(true, bResult);

			pPackageFile.reset();
		}

		void RandomSelect(FileList* outFileList, std::mt19937& gen)
		{
			int nCurFileCount = (*outFileList).size();
			if (nCurFileCount <= 1) return;
			std::uniform_int_distribution<int> distrib(1, nCurFileCount - 1);
			int nSelectFileCount = distrib(gen);

			for (int n = 0; n < nSelectFileCount; n++)
			{
				int nTotalFileCount = (*outFileList).size();
				std::uniform_int_distribution<int> dis(0, nTotalFileCount - 1);
				int nDeleteFile = dis(gen);
				(*outFileList).erase((*outFileList).begin() + nDeleteFile);
			}
		}

		void RandomCopyFile(const std::string& strDir, const std::string& strRandomDir, std::mt19937& gen)
		{
			std::filesystem::copy(strRandomDir, strDir, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

			FileList fileList;
			GetFileListInDirectory(strDir, &fileList);
			RandomSelect(&fileList, gen);
			for (const std::string& strFilename : fileList)
				std::filesystem::remove(strDir + strFilename);
		}

		TEST_METHOD(TestRandomPatch)
		{
			std::random_device rd;
			std::mt19937 gen(rd());

			int nMaxTest = 50;
			for( int n = 0; n < nMaxTest; n++ )
			{
				const std::string STR_RANDOM_DIR = STR_CUR_TEST_DIR + "RandomTest\\";
				const std::string STR_RANDOM_MODIFY_DIR = STR_CUR_TEST_DIR + "RandomModify\\";
				RandomCopyFile(STR_VERSION_1_DIR, STR_RANDOM_DIR, gen);
				RandomCopyFile(STR_VERSION_2_DIR, STR_RANDOM_MODIFY_DIR, gen);
				RandomCopyFile(STR_VERSION_2_DIR, STR_RANDOM_DIR, gen);

				//patch
				TestPatch();
			}
		}
	};
}
