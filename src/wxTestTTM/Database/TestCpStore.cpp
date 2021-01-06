#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"

#include "CpStore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  extern wxString cwd;

	TEST_CLASS(TestCpStore)
	{
		public:

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
		TEST_METHOD(O_010_Import)
		{
			wxMemoryText is;
			is.AddLine("#EVENTS 1");
      is.AddLine("# Name; Description; Type; Sex; Year");
			is.AddLine("MS;Men's Singles;S;M;0");
			bool ret = CpStore::Import(is);
			Assert::IsTrue(ret);

			CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);
		}

		TEST_METHOD(O_011_Import_v2)
		{
			wxMemoryText is;
			is.AddLine("#EVENTS 2");
      is.AddLine("# Name; Description; Category; Type; Sex; Year");
			is.AddLine("MS;Men's Singles;Cat;S;M;0");
			bool ret = CpStore::Import(is);
			Assert::IsTrue(ret);

			CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);
			Assert::AreEqual(wxT("Cat"), cp.cpCategory);
		}

		TEST_METHOD(O_020_Export)
		{
		  wxMemoryText os;
			std::vector<long> ids;
			CpStore::Export(os);

			Assert::AreEqual((size_t) 3, os.GetLineCount());

			wxString line = os.GetFirstLine();
			Assert::AreEqual(line.c_str(), "#EVENTS 2");
			Assert::IsFalse(os.Eof());

			line = os.GetNextLine();
			Assert::IsTrue(line.StartsWith("#"));
			Assert::IsFalse(os.Eof());

			line = os.GetNextLine();
			Assert::AreEqual(line.c_str(), "MS;Men's Singles;Cat;1;1;0;");
		}

	};
}
