/* Copyright (C) 2020 Christoph Theis */

// Datenbankengine

#include  "stdafx.h"

#include  <windows.h>
#include  <sddl.h>

#include   <io.h>
#include   <sys/stat.h>
#include   <direct.h>

#include   <list>
#include   <string>

# include  "TT32App.h"
# include  "TTDbse.h"
# include  "DriverManager.h"
# include  "Connection.h"
# include  "Statement.h"
# include  "SQLException.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "NaStore.h"
#include  "PsStore.h"
#include  "PlStore.h"
#include  "SyStore.h"
#include  "MdStore.h"
#include  "LtStore.h"
#include  "NmStore.h"
#include  "NtStore.h"
#include  "TmStore.h"
#include  "RkStore.h"
#include  "GrStore.h"
#include  "StStore.h"
#include  "MtStore.h"
#include  "XxStore.h"
#include  "UpStore.h"
#include  "RpStore.h"
#include  "PoStore.h"

#include  "CpListStore.h"
#include  "NaListStore.h"
#include  "PlListStore.h"
#include  "LtListStore.h"
#include  "NtListStore.h"
#include  "TmListStore.h"
#include  "SyListStore.h"
#include  "MdListStore.h"
#include  "GrListStore.h"
#include  "StListStore.h"
#include  "RkListStore.h"
#include  "MtListStore.h"
#include  "XxListStore.h"
#include  "UpListStore.h"
#include  "RpListStore.h"

#include  "LtEntryStore.h"
#include  "NtEntryStore.h"
#include  "TmEntryStore.h"
#include  "StEntryStore.h"
#include  "TbEntryStore.h"
#include  "RkEntryStore.h"
#include  "MtEntryStore.h"
#include  "NmEntryStore.h"

#include  "ReportStore.h"

#include  "InfoSystem.h"
#include  "Profile.h"

#include  "Res.h"

#define  DB_VERSION  158

TTDbse * TTDbse::selfPtr = 0;


bool  TTDbse::IsLocalAddress(const wxString &addr)
{
  return 
    addr == "(local)" ||
    addr == "localhost" ||
    addr == "127.0.0.1"
  ;
}

bool  TTDbse::GetWindowsAuthentication(const wxString &connStr)
{
  static wxRegEx authRegEx("Trusted_Connection=([^;]+);");
  return authRegEx.Matches(connStr) && authRegEx.GetMatch(connStr, 1) == "Yes";
}


wxString  TTDbse::GetServer(const wxString &connStr)
{
  static wxRegEx serverRegEx("SERVER=([^;]+);");
  return serverRegEx.Matches(connStr) ? serverRegEx.GetMatch(connStr, 1) : wxEmptyString;
}


wxString  TTDbse::GetDatabase(const wxString &connStr)
{
  static wxRegEx databaseRegEx("DATABASE=([^;]+);");
  return databaseRegEx.Matches(connStr) ? databaseRegEx.GetMatch(connStr, 1) : wxEmptyString;
}


wxString  TTDbse::GetUser(const wxString &connStr)
{
  static wxRegEx userRegEx("UID=([^;]+);");
  return userRegEx.Matches(connStr) ? userRegEx.GetMatch(connStr, 1) : wxEmptyString;
}


wxString  TTDbse::GetPassword(const wxString &connStr)
{
  static wxRegEx passwordRegEx("PWD=([^;]+);");
  return passwordRegEx.Matches(connStr) ? passwordRegEx.GetMatch(connStr, 1) : wxEmptyString;
}


// -----------------------------------------------------------------------

bool  TTDbse::CreateDatabase(const wxString &database, const wxString &server, 
                             bool windowsAuthentication, const wxString &user, const wxString &pwd, 
                             short &type, short &table)
{
  // Try to open database the usual way
  wxString  connStr = 
      windowsAuthentication ?
          wxString::Format("DATABASE=%s;DRIVER=SQL Server;SERVER=%s;Trusted_Connection=Yes;AnsiNPW=No;",
          database.wx_str(), server.wx_str()) :
      wxString::Format("DATABASE=%s;DRIVER=SQL Server;SERVER=%s;Trusted_Connection=No;UID=%s;PWD=%s;AnsiNPW=No;",
          database.wx_str(), server.wx_str(), user.wx_str(), pwd.wx_str())
  ;

  if ( OpenDatabase(connStr, TTDbse::IsLocalAddress(server) ? false : true) )
  {
    type  = IdStore::GetType();
    table = IdStore::GetTable();
    return true;
  }
    
  // Entfernte DB muessen existieren
  if ( !TTDbse::IsLocalAddress(server) )
    return false;
    
  bool exists = false;
  
  // Login to master DB and execute CREATE DATABASE statement
  try
  {
    wxString  masterStr = 
        "DATABASE=master;DRIVER=SQL Server;SERVER=" + server + ";";
    
    if (windowsAuthentication)
      masterStr += "Trusted_Connection=Yes;";
    else
      masterStr += "Trusted_Connection=No;UID=" + user + ";PWD=" + pwd + ";";

    Connection *masterConnection = 
        DriverManager::instance()->GetConnection(masterStr, user, pwd);
        
    // Ohne Masterconnection geht nichts
    if (!masterConnection)
      return false;         

    char *tmp = _getcwd(NULL, _MAX_PATH); 
    wxString cwd(tmp);
    wxString dir = cwd + "\\" + database + "\\";

    // Workaround for wine
    wxString serverPath = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_DATABASEPATH);
    if (!serverPath.IsEmpty())
      serverPath += "/" + database + "/";
    else
      serverPath = dir;

    free(tmp);

    wxString  str;

    // Check if data-file and eventually log-file exist
    if ( wxAccess((dir + "tt32_dat.mdf"), 00) != -1 )
    {
      if ( wxAccess((dir + "tt32_log.ldf"), 00) != -1 )
        str = "sp_attach_db '" + database + "', '" + dir + "tt32_dat.mdf', '" + dir + "tt32_log.ldf'";
      else
        str = "sp_attach_single_file_db '"+ dir + "', '" + dir + "tt32_dat.mdf'";
        
      exists = true;
    }
    else
    {
      // Create directory
      wxMkDir(dir);
      
      str = 
          "CREATE DATABASE " + database +
          " ON (NAME = tt32_dat, FILENAME = '" + serverPath + "tt32_dat.mdf')"
          " LOG ON (NAME = tt32_log, FILENAME = '" + serverPath + "tt32_log.ldf')";
    }

    try
    {
      Statement *stmtPtr = masterConnection->CreateStatement();
      stmtPtr->ExecuteUpdate(str);

      // Change database to snapshot transactions
      str = "ALTER DATABASE " + database + " SET ALLOW_SNAPSHOT_ISOLATION ON";
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER DATABASE " + database + " SET READ_COMMITTED_SNAPSHOT ON";
      stmtPtr->ExecuteUpdate(str);

      delete stmtPtr;

      delete masterConnection;
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      return false;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(wxString("Create database: ") + database, e);
    return false;
  }

  if ( !OpenDatabase(connStr) )
    return false;
    
  if (exists)
  {
    type = IdStore::GetType();
    table = IdStore::GetTable();
  }
  
  return true;
}


bool  TTDbse::OpenDatabase(const wxString &connStr, bool throwError)
{
  try
  {
    defaultConnection = DriverManager::instance()->GetConnection(connStr);
    if (!defaultConnection)
      return false;

    // Nicht per default auf AUTO_COMMIT auf false setzen, 
    // weil sonst jedes SELECT eine Transaktion startet.
    // defaultConnection->SetAutoCommit(false);
  }
  catch (SQLException &e)
  {
    if (throwError)
      infoSystem.Exception(wxString("Open database ") + connStr, e);
    return false;
  }

  if (!defaultConnection)
    return false;

  strDsn  = defaultConnection->GetConnectionString();

  // Update DB-Tables
  if (!UpdateTables(IdStore::IdVersion()))
    return true;   // XXX What to do?
    
  return true;
}


// Detach database 
bool  TTDbse::DetachDatabase(const wxString &connStr, bool throwError)
{
  wxString dbse, server, uid, pwd;
  bool auth;
  char *copy = (char *) _alloca(connStr.size() + 1);
  wxStrcpy(copy, connStr.data());
  
  dbse =   TTDbse::GetDatabase(connStr);
  server = TTDbse::GetServer(connStr);
  uid =    TTDbse::GetUser(connStr);
  pwd =    TTDbse::GetPassword(connStr);
  auth =   TTDbse::GetWindowsAuthentication(connStr);

  // Keine lokale Datenbank, also auch kein wirkliches Detach machen
  // "Loeschen" ist aber erfolgreich, das Turnier wird aus dem Ini-File geworfen
  if ( !TTDbse::IsLocalAddress(server) )
    return true;
  
  // Kein DB konnte ermittelt werden? Also kann sie auch nicht geloescht werden
  if (!dbse)
    return false;
    
  // Login to master DB and execute CREATE DATABASE statement
  try
  {
    wxString  masterStr = 
        "DATABASE=master;DRIVER=SQL Server;SERVER=" + wxString(server) + ";";
    
    if (auth)
      masterStr += "Trusted_Connection=Yes;";
    else
      masterStr += "Trusted_Connection=No;UID=" + uid + ";PWD=" + pwd + ";";

    Connection *masterConnection = 
        DriverManager::instance()->GetConnection(masterStr);

    if (masterConnection)
    {
      wxString  str = "sp_detach_db '";
      str += dbse;
      str += "', 'true'";

      Statement *stmtPtr = masterConnection->CreateStatement();
      stmtPtr->ExecuteUpdate(str);
      delete stmtPtr;
    }

    delete masterConnection;
  }
  catch (SQLException &e)
  {
    if (throwError)
      infoSystem.Exception(wxString("Detach dabase: ") + dbse, e);
    return false;
  }
  
  return true;
}


// -----------------------------------------------------------------------
// Create DB Roles
bool  TTDbse::CreateRoles()
{
  // return true;
  
  wxString str;
  Statement *stmtPtr = defaultConnection->CreateStatement();
  
  wxString users;
  wxString powerusers;
  wxString admins;
  
  DWORD cbName = MAX_PATH-1, cbDomainName = MAX_PATH-1;
  TCHAR builtinUsers[MAX_PATH], builtinPowerUsers[256], builtinAdmins[256], domainName[MAX_PATH];
  SID_NAME_USE peUse;
  
  PSID sid = NULL;
  
  // S-1-5-32-544: BUILTIN\Admins
  if (ConvertStringSidToSid(_T("S-1-5-32-544"), &sid) &&
      LookupAccountSid(NULL, sid, builtinAdmins, &cbName, domainName, &cbDomainName, &peUse))
  {
    admins = wxString(domainName) + "\\" + builtinAdmins;
    LocalFree(sid);
  }
  
  sid = NULL;
  cbName = cbDomainName = MAX_PATH - 1;  
  
  // S-1-5-32-547: BUILTIN\Power Users
  if (ConvertStringSidToSid(_T("S-1-5-32-547"), &sid) &&
      LookupAccountSid(NULL, sid, builtinPowerUsers, &cbName, domainName, &cbDomainName, &peUse))
  {
    powerusers = wxString(domainName) + "\\" + builtinPowerUsers;
    LocalFree(sid);
  }
    
  sid = NULL;
  cbName = cbDomainName = MAX_PATH - 1;  
  
  // S-1-5-32-545: BUILTIN\Users
  if (ConvertStringSidToSid(_T("S-1-5-32-545"), &sid) &&
      LookupAccountSid(NULL, sid, builtinUsers, &cbName, domainName, &cbDomainName, &peUse))
  {
    users = wxString(domainName) + "\\" + builtinUsers;
    LocalFree(sid);
  }
    
  try
  {
    // Default SQL Rollen
    stmtPtr->ExecuteUpdate(str = "CREATE ROLE ttm_results AUTHORIZATION db_datareader");

    stmtPtr->ExecuteUpdate(str = "GRANT EXECUTE ON mtSetResultProc TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT EXECUTE ON mtUpdateRasterProc TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT UPDATE ON MtRec (mtDateTime, mtTable, mtUmpire, mtUmpire2, mtPrinted, mtChecked, mtReverse, mtBestOf) TO ttm_results");    
    stmtPtr->ExecuteUpdate(str = "GRANT UPDATE ON MtRec (mtWalkOverA, mtWalkOverX, mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX) TO ttm_results");    
    stmtPtr->ExecuteUpdate(str = "GRANT UPDATE On StRec (stNoCons) TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT UPDATE ON IdRec (idLast) TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT INSERT, DELETE ON NmSingle TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT INSERT, DELETE ON NmDouble TO ttm_results");
    stmtPtr->ExecuteUpdate(str = "GRANT INSERT, DELETE ON NmRec TO ttm_results");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  try 
  {
    if (users.size() > 0)
    {
      stmtPtr->ExecuteUpdate(str = "CREATE USER [" + users + "] FOR LOGIN [" + users + "]");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_datareader', '" + users + "'");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_backupoperator', '" + users + "'");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'ttm_results', '" + users + "'");
    }
    
    if (powerusers.size() > 0)
    {
      stmtPtr->ExecuteUpdate(str = "CREATE USER [" + powerusers + "] FOR LOGIN [" + powerusers + "]");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_datareader', '" + powerusers + "'");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_datawriter', '" + powerusers + "'");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_backupoperator', '" + powerusers + "'");
    }
    
    if (admins.size() > 0)
    {
      stmtPtr->ExecuteUpdate(str = "CREATE USER [" + admins + "] FOR LOGIN [" + admins + "]");
      stmtPtr->ExecuteUpdate(str = "sp_addrolemember 'db_owner', '" + admins + "'");
    }
  }
  catch (SQLException &)
  {
    // Koennen fehlschlagen, wenn es keine entsprechenden User gibt (z.B. Wine)
    // infoSystem.Exception(str, e);
  }
  
  delete stmtPtr;
  
  return true;
}

// Create DB-Domains (entsprechend ACCESS)
bool  TTDbse::CreateDomains()
{
  return true;
}


// Create DB-Tables
bool  TTDbse::CreateTables()
{
  // Create Domains
  CreateDomains();

  // Create Tables
  IdStore::CreateTable(DB_VERSION);
  CpStore::CreateTable();
  NaStore::CreateTable();
  PsStore::CreateTable();
  PlStore::CreateTable();
  SyStore::CreateTable();
  MdStore::CreateTable();
  LtStore::CreateTable();
  NtStore::CreateTable();
  TmStore::CreateTable();
  RkStore::CreateTable();
  GrStore::CreateTable();
  StStore::CreateTable();
  MtStore::CreateTable();
  NmStore::CreateTable();
  XxStore::CreateTable();
  UpStore::CreateTable();
  RpStore::CreateTable();
  PoStore::CreateTable();

  // Create Constraints
  MdStore::CreateConstraints();
  SyStore::CreateConstraints();
  PlStore::CreateConstraints();
  LtStore::CreateConstraints();
  NtStore::CreateConstraints();
  TmStore::CreateConstraints();
  RkStore::CreateConstraints();
  CpStore::CreateConstraints();
  GrStore::CreateConstraints();
  StStore::CreateConstraints();
  MtStore::CreateConstraints();
  NmStore::CreateConstraints();
  UpStore::CreateConstraints();
  RpStore::CreateConstraints();

  // Create Views
  CpListStore::CreateView();
  NaListStore::CreateView();
  PlListStore::CreateView();
  LtListStore::CreateView();
  NtListStore::CreateView();
  TmListStore::CreateView();
  SyListStore::CreateView();
  MdListStore::CreateView();
  GrListStore::CreateView();
  StListStore::CreateView();
  RkListStore::CreateView();
  MtListStore::CreateView();
  XxListStore::CreateView();
  UpListStore::CreateView();
  RpListStore::CreateView();

  LtEntryStore::CreateView();
  NmEntryStore::CreateView();
  NtEntryStore::CreateView();
  TmEntryStore::CreateView();
  StEntryStore::CreateView();
  TbEntryStore::CreateView();
  RkEntryStore::CreateView();
  MtEntryStore::CreateView();
  
  ReportStore::CreateView();

  // Create Roles
  CreateRoles();
  
  return true;
}


// Update DB-Tables
bool  TTDbse::UpdateTables(long version)
{
  bool res = true;
  
  if (version == 0)
    res = CreateTables();
  else if (version < DB_VERSION)
  {
    // Update Tables
    res &= IdStore::UpdateTable(version, DB_VERSION);
    res &= CpStore::UpdateTable(version);
    res &= NaStore::UpdateTable(version);
    res &= PsStore::UpdateTable(version);
    res &= PlStore::UpdateTable(version);
    res &= SyStore::UpdateTable(version);
    res &= MdStore::UpdateTable(version);
    res &= LtStore::UpdateTable(version);
    res &= NtStore::UpdateTable(version);
    res &= TmStore::UpdateTable(version);
    res &= RkStore::UpdateTable(version);
    res &= GrStore::UpdateTable(version);
    res &= StStore::UpdateTable(version);
    res &= MtStore::UpdateTable(version);
    res &= NmStore::UpdateTable(version);
    res &= XxStore::UpdateTable(version);
    res &= UpStore::UpdateTable(version);
    res &= RpStore::UpdateTable(version);
    res &= PoStore::UpdateTable(version);

    // Update Constraints
    res &= MdStore::UpdateConstraints(version);
    res &= SyStore::UpdateConstraints(version);
    res &= PlStore::UpdateConstraints(version);
    res &= LtStore::UpdateConstraints(version);
    res &= NtStore::UpdateConstraints(version);
    res &= TmStore::UpdateConstraints(version);
    res &= RkStore::UpdateConstraints(version);
    res &= CpStore::UpdateConstraints(version);
    res &= GrStore::UpdateConstraints(version);
    res &= StStore::UpdateConstraints(version);
    res &= MtStore::UpdateConstraints(version);
    res &= NmStore::UpdateConstraints(version);
    res &= UpStore::UpdateConstraints(version);
    res &= RpStore::UpdateConstraints(version);

    // Update Views
    res &= ReportStore::RemoveView();
    
    res &= MtEntryStore::RemoveView();
    res &= RkEntryStore::RemoveView();
    res &= TbEntryStore::RemoveView();
    res &= StEntryStore::RemoveView();
    res &= TmEntryStore::RemoveView();
    res &= NtEntryStore::RemoveView();
    res &= NmEntryStore::RemoveView();
    res &= LtEntryStore::RemoveView();
    
    res &= RpListStore::RemoveView();
    res &= UpListStore::RemoveView();
    res &= XxListStore::RemoveView();
    res &= MtListStore::RemoveView();
    res &= RkListStore::RemoveView();
    res &= StListStore::RemoveView();
    res &= GrListStore::RemoveView();
    res &= MdListStore::RemoveView();
    res &= SyListStore::RemoveView();
    res &= TmListStore::RemoveView();
    res &= NtListStore::RemoveView();
    res &= LtListStore::RemoveView();
    res &= PlListStore::RemoveView();
    res &= NaListStore::RemoveView();
    res &= CpListStore::RemoveView();
    
    res &= CpListStore::CreateView();
    res &= NaListStore::CreateView();
    res &= PlListStore::CreateView();
    res &= LtListStore::CreateView();
    res &= NtListStore::CreateView();
    res &= TmListStore::CreateView();
    res &= SyListStore::CreateView();
    res &= MdListStore::CreateView();
    res &= GrListStore::CreateView();
    res &= StListStore::CreateView();
    res &= RkListStore::CreateView();
    res &= MtListStore::CreateView();
    res &= XxListStore::CreateView();
    res &= UpListStore::CreateView();
    res &= RpListStore::CreateView();
    
    res &= LtEntryStore::CreateView();
    res &= NmEntryStore::CreateView();
    res &= NtEntryStore::CreateView();
    res &= TmEntryStore::CreateView();
    res &= StEntryStore::CreateView();
    res &= TbEntryStore::CreateView();
    res &= RkEntryStore::CreateView();
    res &= MtEntryStore::CreateView();
    
    res &= ReportStore::CreateView();    
  }

  return res;
}


// -----------------------------------------------------------------------
Connection * TTDbse::GetDefaultConnection() 
{
  if (!defaultConnection)
    return 0;

  if (defaultConnection->IsValid())
    return defaultConnection;

  try 
  {
    defaultConnection->Close();
  }
  catch (SQLException &)
  {
    // Nothing
  }

  Connection *conn = GetNewConnection();
  if (!conn)
    return 0;

  delete defaultConnection;
  defaultConnection = conn;

  return defaultConnection;
}

// -----------------------------------------------------------------------
Connection * TTDbse::GetNewConnection()
{
  if (!defaultConnection)
    return 0;

  try
  {
    Connection * connPtr = DriverManager::instance()->
        GetConnection(strDsn, strUser, strPwd);

    // Nicht per Default AUTO_COMMIT auf false setzen,
    // weil sonst jedes SELECT eine Transaktion startet
    // connPtr->SetAutoCommit(false);

    return connPtr;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(wxString("New Connection "), e);
    return 0;
  }
}


// -----------------------------------------------------------------------
std::list<wxString> TTDbse::ListServers() const
{
  std::list<wxString> result;
  
  HENV hEnv = SQL_NULL_HENV;
  HDBC hDbc = SQL_NULL_HDBC;
  
  SQLSMALLINT nBufSize = 30000;    
	SQLSMALLINT nReqBufSize = 0;
	TCHAR	*lpFetchBuf = new TCHAR[nBufSize];
	memset(lpFetchBuf, 0, nBufSize * sizeof(TCHAR));

  // __try / __finally ist mit std::list nicht moeglich.
  if (SQLAllocEnv(&hEnv) == SQL_ERROR)
    goto exit;
    
  if (SQLAllocConnect(hEnv, &hDbc) == SQL_ERROR)
    goto exit;
    
  if (SQLSetConnectAttr(hDbc, SQL_COPT_SS_BROWSE_CONNECT, 
        (SQLPOINTER) SQL_MORE_INFO_NO, SQL_IS_UINTEGER) == SQL_ERROR)
    goto exit;

  // Fuer grosse Netzwerke    
  if (false && SQLSetConnectAttr(hDbc, SQL_COPT_SS_BROWSE_CACHE_DATA, 
        (SQLPOINTER) SQL_CACHE_DATA_YES, SQL_IS_INTEGER) == SQL_ERROR)
    goto exit;

  // Eigentlich muesste man den Fall behandeln, dass der Buffer nicht ausreicht 
  // SQLBrowseConnect hat einen Bug, fuer den es aber von MS nur einen Hotfix gibt.
  try 
  {   
	  if (SQLBrowseConnect(hDbc, (SQLTCHAR *) _T("Driver=SQL Server"), SQL_NTS,
	        (SQLTCHAR *) lpFetchBuf, nBufSize, &nReqBufSize) == SQL_NEED_DATA )
	  {
	      TCHAR *start = _tcschr(lpFetchBuf, '{');
	      TCHAR *end   = (start ? _tcschr(start, '}') : NULL);
  	    
	      for (TCHAR *ptr = start + 1; ptr < end; ptr++)
	      {
	        if (*ptr == ',')
	        {
	          *ptr = '\0';
	          result.push_back(wxString(start+1));
	          start = ptr;
	          ptr = start + 1;
	        }
	      }
	  }
	}
	catch (...) 
	{
	  // Nothing
	}
    
exit:    
  
  if (hDbc != SQL_NULL_HDBC)
    SQLFreeConnect(&hDbc);
    
  if (hEnv != SQL_NULL_HENV)
    SQLFreeEnv(&hEnv);      
    
  delete[] lpFetchBuf;
  
  return result;
}


// -----------------------------------------------------------------------
std::list<wxString> TTDbse::ListDatabases(const wxString &server, const wxString &connStr) const
{
  std::list<wxString>  result;
  
  wxString  masterStr = connStr.IsEmpty() ?
      "DRIVER=SQL Server;SERVER="+server+";Trusted_Connection=Yes;" : connStr;
      
  Connection *masterConnection = 0;
  Statement  *stmtPtr = 0;
  ResultSet  *resPtr = 0;
  
  try
  {    
    masterConnection = DriverManager::instance()->GetConnection(masterStr, "", "");
  }
  catch(SQLException &e)
  {
	  infoSystem.Exception("Connect to master database", e);
    return result;
  }

  if (masterConnection == NULL)
  {
    return result;
  }
    
  try
  {
    stmtPtr = masterConnection->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery("SELECT name FROM sys.databases");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception("SELECT name FROM sys.databases", e);
    resPtr = 0;
  }
  
  if (resPtr)
  {
    while (resPtr->Next())
    {
      wxChar  tmp[100];
      if (resPtr->GetData(1, tmp, 100))
      {
        // Ausnahmen definieren
        if (false && !wxStrcmp(tmp, wxT("master")))
          continue;
        if (!wxStrcmp(tmp, wxT("model")))
          continue;
        if (!wxStrcmp(tmp, wxT("msdb")))
          continue;
        if (!wxStrcmp(tmp, wxT("pubs")))
          continue;
        if (!wxStrcmp(tmp, wxT("tempdb")))
          continue;
        if (!wxStrcmp(tmp, wxT("Northwind")))
          continue;

        // ReportServer databases
        if (!wxStrcmp(tmp, wxT("ReportServer")))
          continue;
        if (!wxStrcmp(tmp, wxT("ReportServerTempDB")))
          continue;

        result.push_back(wxString(tmp));
      }
    }
  }
  
  delete resPtr;
  delete stmtPtr;
  delete masterConnection;
  
  return result; 
}

// -----------------------------------------------------------------------
bool  TTDbse::BackupDatabase(const wxString &fileName)
{
  if (!defaultConnection)  
    return false;
    
  wxChar *savePtr = NULL;
  wxChar *tmp = wxStrdup(strDsn.wx_str());
  wxChar *ptr = wxStrtok(tmp, wxT(";"), &savePtr);
  while (ptr && wxStrncmp("DATABASE=", ptr, wxStrlen("DATABASE=")))
    ptr = wxStrtok((wxChar *) NULL, wxT(";"), &savePtr);
    
  if (ptr == NULL)
    return false;
    
  wxString name = (ptr + wxStrlen("DATABASE="));
  free(tmp);
    
  Statement *stmt = defaultConnection->CreateStatement();
  
  wxString str = 
  "BACKUP DATABASE " + name + " TO DISK = '" + fileName + "' WITH INIT, SKIP";
 
  try
  {
    stmt->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    
    delete stmt;
    return false;
  } 
    
  delete stmt;
  return true;
}


bool  TTDbse::RestoreDatabase(const wxString &fileName, const wxString &dir)
{
  wxChar *savePtr = NULL;
  wxChar *tmp = wxStrdup(strDsn.wx_str());
  wxChar *ptr = wxStrtok(tmp, wxT(";"), &savePtr);
  while (ptr && wxStrncmp("DATABASE=", ptr, wxStrlen("DATABASE=")))
    ptr = wxStrtok(NULL, wxT(";"), &savePtr);
    
  if (ptr == NULL)
    return false;
    
  wxString name = (ptr + wxStrlen("DATABASE="));
  free(tmp);

  // RESTORE braucht exclusiv access, daher die Defaultverbindung
  // schliessen. Wenn weitere Verbindungen offen sind, gibt es 
  // eine Fehlermeldung. 
  if (defaultConnection)
    defaultConnection->Close();
  
  delete defaultConnection;
  
  defaultConnection = NULL;
  
  wxString  masterStr = 
    "DATABASE=master;DRIVER=SQL Server;SERVER=(local);Trusted_Connection=Yes;";

  Connection *masterConnection = 
    DriverManager::instance()->GetConnection(masterStr, strUser, strPwd);

  if (!masterConnection)  
    return false;
    
  Statement *stmt = masterConnection->CreateStatement();
  
  char *cwd = _getcwd(NULL, _MAX_PATH);
  wxString path = wxString(cwd) + "\\" + dir;
  
  free(cwd);
  
  wxString str = 
  "RESTORE DATABASE " + name + " FROM DISK = '" + fileName + "' " // WITH REPLACE
  "WITH MOVE 'tt32_dat' TO '" + path + "\\tt32_dat.mdf', "
  "     MOVE 'tt32_log' TO '" + path + "\\tt32_log.ldf', "
  "     REPLACE";
 
  try
  {
    stmt->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    
    delete stmt;
    return false;
  } 
    
  delete stmt;

  // Datenbank "offiziell" oeffnen, um das Schema nachzuziehen
  return OpenDatabase(strDsn, true);
}


wxString TTDbse::GetServer() const
{
  return GetDsnPart("SERVER");
}


wxString TTDbse::GetDatabase() const
{
  return GetDsnPart("DATABASE");
}

bool TTDbse::IsWindowsAuthenticaton() const
{
  return GetDsnPart("TRUSTED_CONNECTION").IsSameAs("yes", false);
}

wxString TTDbse::GetUsername() const
{
  return GetDsnPart("UID");
}

wxString TTDbse::GetPassword() const
{
  return GetDsnPart("PWD");
}


wxString TTDbse::GetDsnPart(const wxString &part) const
{
  wxString search = part.Upper() + "=";
  wxArrayString parts = wxStringTokenize(strDsn, ";");

  for (unsigned idx = 0; idx < parts.GetCount(); idx++)
  {
    if (parts[idx].Upper().StartsWith(search))
      return parts[idx].SubString(search.Length(), -1);
  }

  return "";
}
