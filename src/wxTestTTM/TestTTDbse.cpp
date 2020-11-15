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
    wxSetWorkingDirectory("F:\\User\\ChT\\TTM");

    wxString connStr = "DRIVER=SQL Server;SERVER=localhost;UID=chtheis;Trusted_Connection=Yes;WSID=SAURON;DATABASE=TTM_UTEST;AnsiNPW=No;";
    bool ret = TTDbse::instance()->OpenDatabase(connStr, false);
    Assert::IsTrue(ret);
  }
}