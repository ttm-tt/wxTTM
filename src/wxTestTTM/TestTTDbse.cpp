#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  // Rune once before all tests
  TEST_MODULE_INITIALIZE(SetUpTestTTM)
  {
    // TODO: Somehow configurable
    wxSetWorkingDirectory("..\\..\\..\\..\\..\\TTM");

    wxString connStr = "DRIVER=SQL Server;SERVER=localhost;Trusted_Connection=Yes;DATABASE=TTM_UTEST;AnsiNPW=No;";
    bool ret = TTDbse::instance()->OpenDatabase(connStr, false);
    Assert::IsTrue(ret);
  }
}