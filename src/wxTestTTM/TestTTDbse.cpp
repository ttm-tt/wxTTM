#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"
#include "DriverManager.h"
#include "Statement.h"
#include "ResultSet.h"
#include "SQLException.h"
#include "InfoSystem.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  // Rune once before all tests
  TEST_MODULE_INITIALIZE(SetUpTestTTM)
  {
    wxString masterStr = "DRIVER=SQL Server;SERVER=localhost;Trusted_Connection=Yes;";
    wxString sql = 
      "SELECT sys.master_files.physical_name "
      "  FROM sys.databases INNER JOIN sys.master_files ON sys.databases.database_id = sys.master_files.database_id "
      " WHERE sys.databases.name = 'TTM_UTEST' AND type_desc = 'ROWS'"
    ;

    Connection *masterConnection = 0;
    Statement  *stmtPtr = 0;
    ResultSet  *resPtr = 0;
  
    masterConnection = DriverManager::instance()->GetConnection(masterStr, "", "");

    Assert::IsNotNull(masterConnection);
    
    stmtPtr = masterConnection->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(sql);
  
    wxChar  tmp[100];
    Assert::IsTrue(
        resPtr != NULL && resPtr->Next() && resPtr->GetData(1, tmp, 100), 
        wxT("Could not find tournament / database \"TTM_UTEST\", please create it before running the tests")
    );
    
    delete resPtr;
    delete stmtPtr;
    delete masterConnection;

    wxString cd = wxFileName(wxFileName(tmp).GetPath()).GetPath();
    wxSetWorkingDirectory(cd);

    wxString connStr = "DRIVER=SQL Server;SERVER=localhost;Trusted_Connection=Yes;DATABASE=TTM_UTEST;AnsiNPW=No;";
    bool ret = TTDbse::instance()->OpenDatabase(connStr, false);
    Assert::IsTrue(ret);
  }
}