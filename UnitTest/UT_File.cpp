#include "pch.h"
#include "CppUnitTest.h"
#include "../FilePackageManager/File.h"
#include<iostream>
#include<vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const std::string STR_CUR_TEST_DIR = STR_TEST_RESOURCE + "TestFile\\";

namespace UT_File
{
	TEST_CLASS(UT_File)
	{
	private:
	public:
		UT_File()
		{}

		TEST_METHOD_INITIALIZE(MethodInitialize)
		{
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		}

	public:
		TEST_METHOD(TestFileSaveAndLoad)
		{
			std::unique_ptr<CFile> m_pSaveFile = std::make_unique<CFile>(STR_CUR_TEST_DIR + "TestSaveAndLoad.cca", eFileType::WRITE);
			__int64 nIn = 31424156788567;
			std::string strIn = "good test 1234 Ä¼Ä¼ !@#$!@#¤½¤¤½îdFQDZ<>?";
			m_pSaveFile->Write(nIn);
			m_pSaveFile->Write(strIn);
			m_pSaveFile->Close();

			std::unique_ptr<CFile> m_pLoadFile = std::make_unique<CFile>(STR_CUR_TEST_DIR + "TestSaveAndLoad.cca", eFileType::READ);
			__int64 nOut = 0;
			std::string strOut;
			m_pLoadFile->Read(&nOut);
			m_pLoadFile->Read(&strOut);
			m_pLoadFile->Close();

			Assert::AreEqual(true, nIn == nOut);
			Assert::AreEqual(true, strIn == strOut);
		}
		//ÀÐ¾î¼­ ÆÄÀÏºñ±³¸¦ ÇØ ÁØ´Ù.
		TEST_METHOD(TestFileMemoryLoad)
		{
			bool bResult = false;

			std::unique_ptr<CFile> m_pFile = std::make_unique<CFile>(STR_CUR_TEST_DIR + "test1.txt", eFileType::READ);
			stReadData readData;
			bResult = m_pFile->Read(0, &readData);
			Assert::AreEqual(true, readData.nSize != 0);

			std::unique_ptr<CFile> m_pDiffFile = std::make_unique<CFile>(STR_CUR_TEST_DIR + "test1.txt", eFileType::READ);
			bResult = m_pFile->IsEqual(m_pDiffFile.get());
			Assert::AreEqual(true, bResult);

			m_pFile->Close();
			m_pDiffFile->Close();
		}
	};
}
