/* Copyright (C) 2020 Christoph Theis */

// Implementation of the DriverManager

# include  "stdafx.h"

# include  "DriverManager.h"
# include  "Connection.h"
# include  "Statement.h"
# include  "SQLException.h"

# include  "InfoSystem.h"
extern InfoSystem infoSystem;

// Singleton: selfPtr
DriverManager * DriverManager::selfPtr = NULL;


// Constructor
DriverManager::DriverManager()
{
  SQLRETURN  ret;

  hEnv = SQL_NULL_HANDLE;

  ret = SQLAllocEnv(&hEnv);
  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, SQL_NULL_HANDLE, hEnv);
}


DriverManager::~DriverManager()
{
  SQLFreeEnv(hEnv);
}


// Create new connection object
Connection * DriverManager::GetConnection(
  const wxString &dsn, const wxString &uid, const wxString &pwd)
{
  wxString  connString = dsn;
  if (uid.length())
    connString += "USER="+uid+";";
  if (pwd.length())
    connString += "PWD="+pwd+";";

  // Verbindung ueber "SQL Server Native Client 10.0" herstellen
  // connString.Replace("DRIVER=SQL Server;", "DRIVER=SQL Server Native Client 10.0;");

  return GetConnection(connString);
}


// To clone a connection
Connection * DriverManager::GetConnection(const wxString &connStr)
{
  SQLHDBC    hDbc = SQL_NULL_HANDLE;
  SQLRETURN  ret;

  // Allocate connection handle
	ret = SQLAllocConnect(hEnv, &hDbc); 
  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, SQL_NULL_HANDLE, hEnv);

	// Set login timeout to 10 seconds, ignore errors
	ret = SQLSetConnectOption(hDbc, SQL_LOGIN_TIMEOUT, (UDWORD) 30);
  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, hDbc, hEnv);

  wxChar connStrOut[1024];
  short  connStrOutLen;

  // Test, ob SQL Server Native Client installiert ist und den stattdessen verwenden.
  // Der normale SQL Server ODBC-Treiber kann sehr langsam sein.
 static  wxRegEx driverRegEx("DRIVER=[^;]+;");
 static  wxRegEx serverRegEx("SERVER=\\(local\\);");

  wxString tmp = connStr;
  HKEY hkResult;

  // Replace (local) with localhost
  serverRegEx.Replace(&tmp, "SERVER=localhost;");

  // No UID and PWD allowed if this is a trusted connection
  if (tmp.Contains("Trusted_Connection=Yes;"))
  {
	  wxRegEx("UID=[^;]*;").Replace(&tmp, "");
	  wxRegEx("PWD=[^;]*;").Replace(&tmp, "");
  }

  if (false)
	  ;
  else if (RegOpenKey(HKEY_LOCAL_MACHINE, wxT("Software\\ODBC\\ODBCINST.INI\\ODBC Driver 17 for SQL Server"), &hkResult) == ERROR_SUCCESS)
  {
    RegCloseKey(hkResult);
    driverRegEx.Replace(&tmp, "DRIVER=ODBC Driver 17 for SQL Server;");
  }
  else if (RegOpenKey(HKEY_LOCAL_MACHINE, wxT("Software\\ODBC\\ODBCINST.INI\\ODBC Driver 13 for SQL Server"), &hkResult) == ERROR_SUCCESS)
  {
	  RegCloseKey(hkResult);
	  driverRegEx.Replace(&tmp, "DRIVER=ODBC Driver 13 for SQL Server;");
  }
  else if (RegOpenKey(HKEY_LOCAL_MACHINE, wxT("Software\\ODBC\\ODBCINST.INI\\SQL Server Native Client 11.0"), &hkResult) == ERROR_SUCCESS)
  {
	  RegCloseKey(hkResult);
	  driverRegEx.Replace(&tmp, "DRIVER=SQL Server Native Client 11.0;");
  }
  else if (RegOpenKey(HKEY_LOCAL_MACHINE, wxT("Software\\ODBC\\ODBCINST.INI\\SQL Server Native Client 10.0"), &hkResult) == ERROR_SUCCESS)
  {
    RegCloseKey(hkResult);
	  driverRegEx.Replace(&tmp, "DRIVER=SQL Server Native Client 10.0;");
  }

  ret = SQLDriverConnect(
    hDbc, (SQLHWND) (wxTheApp ? wxTheApp->GetTopWindow()->GetHWND() : NULL),
    (SQLTCHAR *) tmp.wx_str(), SQL_NTS, (SQLTCHAR *) connStrOut, 1023, &connStrOutLen, SQL_DRIVER_COMPLETE_REQUIRED);

  // Cancel beim Loginfenster
  if (ret == SQL_NO_DATA)
    return 0;

  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, hDbc, hEnv);    

  tmp = connStrOut;
  driverRegEx.Replace(&tmp, "DRIVER=SQL Server;");
  return new Connection(hEnv, hDbc, tmp);
}


// -----------------------------------------------------------------------
// Enumerate Datasource
bool  DriverManager::GetFirstDataSource(wxString &strDbse, wxString &strDesc)
{
# if 1
  SQLRETURN  ret;

  wxChar  dbse[SQL_MAX_DSN_LENGTH];
  wxChar  desc[SQL_MAX_DSN_LENGTH];

  SWORD lenDbse, lenDesc;

  ret = SQLDataSources(hEnv, SQL_FETCH_FIRST, 
                       (SQLTCHAR *) dbse, SQL_MAX_DSN_LENGTH, &lenDbse, 
                       (SQLTCHAR *) desc, SQL_MAX_DSN_LENGTH, &lenDesc);

  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, SQL_NULL_HANDLE, hEnv);
  else if (ret == SQL_NO_DATA_FOUND)
    return false;

  strDbse.assign(dbse);
  strDesc.assign(desc);

  return true;
# else
  return false;
# endif
}


bool  DriverManager::GetNextDataSource(wxString &strDbse, wxString &strDesc)
{
# if 1
  SQLRETURN  ret;

  wxChar  dbse[256];
  wxChar  desc[256];

  SWORD lenDbse, lenDesc;

  ret = SQLDataSources(hEnv, SQL_FETCH_NEXT, 
                       (SQLTCHAR *) dbse, 255, &lenDbse, 
                       (SQLTCHAR *) desc, 255, &lenDesc);

  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, SQL_NULL_HANDLE, hEnv);
  else if (ret == SQL_NO_DATA_FOUND)
    return false;

  strDbse.assign(dbse);
  strDesc.assign(desc);

  return true;
# else
  return false;
# endif
}


