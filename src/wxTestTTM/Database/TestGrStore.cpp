#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"

#include "GrStore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  extern wxString cwd;

	TEST_CLASS(TestGrStore)
	{
		public:

			// Run once before all methods in this class
			TEST_CLASS_INITIALIZE(SetUpClassGrStore)
			{
		    wxSetWorkingDirectory(cwd);
	
				// Reset DB to known state
				wxFileName tmpName("TTM_UTEST", "TTM_UTEST-v153.bak");
				tmpName.Normalize();
  
				bool ret = TTDbse::instance()->RestoreDatabase(tmpName.GetFullPath(), "TTM_UTEST");
				Assert::IsTrue(ret);				
			}
		
		TEST_METHOD(O_010_TestNofRounds)
		{
		  // RR group of 4
			{
				GrRec gr;
				gr.grModus = MOD_RR;
				gr.grSize = 4;

				Assert::AreEqual((short) 3, gr.NofRounds(false));
		  }

			// KO Group of 4
			{
				GrRec gr;
				gr.grModus = MOD_SKO;
				gr.grSize = 4;

				Assert::AreEqual((short) 2, gr.NofRounds(false));
		  }
		}

		TEST_METHOD(O_020_Import)
		{

		}

		TEST_METHOD(O_030_Export)
		{
		  wxMemoryText os;
			std::vector<long> ids;
			GrStore::Export(os, CP_SINGLE, ids, false);

			wxString line = os.GetFirstLine();
			Assert::AreEqual(line.c_str(), "#GROUPS 1");
		}
	};
}
