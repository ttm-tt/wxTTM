#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"

#include "CpStore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
	TEST_CLASS(TestCpStore)
	{
		public:

		// Run once before all methods in this class
		TEST_CLASS_INITIALIZE(SetUpClassCpStore)
		{
			// Reset DB to known state
			wxFileName tmpName("TTM_UTEST", "TTM_UTEST-v153.bak");
			tmpName.Normalize();
  
			bool ret = TTDbse::instance()->RestoreDatabase(tmpName.GetFullPath(), "TTM_UTEST");
			Assert::IsTrue(ret);		
		}

		TEST_METHOD(Export)
		{
		  wxMemoryText os;
			std::vector<long> ids;
			CpStore::Export(os);

			wxString line = os.GetFirstLine();
			Assert::AreEqual(line.c_str(), "#EVENTS 1");
		}
	};
}
