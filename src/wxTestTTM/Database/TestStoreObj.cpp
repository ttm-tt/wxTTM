#include "pch.h"

#include <CppUnitTest.h>

#include "StoreObj.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  extern wxString cwd;

	TEST_CLASS(TestStoreObj)
  {
		// Run once before all methods in this class
		TEST_CLASS_INITIALIZE(SetUpClassCpStore)
		{
	    wxSetWorkingDirectory(cwd);

			// Reset DB to known state
			wxFileName tmpName("TTM_UTEST", "TTM_UTEST-v153.bak");
			tmpName.Normalize();
  
			bool ret = TTDbse::instance()->RestoreDatabase(tmpName.GetFullPath(), "TTM_UTEST");
			Assert::IsTrue(ret);		
		}

		// To run tests in order name them alphabetically
		// Valid headers
		TEST_METHOD(O_010_CheckImportHeader_Valid)
		{
		  {
				// Correct header v2
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("#EVENTS 2", "#EVENTS", version);
				Assert::IsTrue(ret);
				Assert::AreEqual(2L, version);
			}

		  {
				// Correct header v!
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("#EVENTS 1", "#EVENTS", version);
				Assert::IsTrue(ret);
				Assert::AreEqual(1L, version);
			}

		  {
				// Correct header no version
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("#EVENTS", "#EVENTS", version);
				Assert::IsTrue(ret);
				Assert::AreEqual(1L, version);
			}
			
		}

		// Invalid headers
		TEST_METHOD(O_020_CheckImportHeader_Invalid)
		{
		  {
				// Invalid magic
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("#EVENTS", "#GROUPS", version);
				Assert::IsFalse(ret);
			}

		  {
				// non-numeric version
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("#EVENTS X", "#EVENTS", version);
				Assert::IsFalse(ret);
			}

		  {
				// Surplus space in header
				long version = 0;
				bool ret = StoreObj::CheckImportHeader("# EVENTS", "#EVENTS", version);
				Assert::IsFalse(ret);
			}
		}
  };
}
