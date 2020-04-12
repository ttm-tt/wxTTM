/* Copyright (C) 2020 Christoph Theis */

// Implementation of the connection class

#include  "stdafx.h"

#include  "DriverManager.h"
#include  "Connection.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"


// Constructor, argument is the connection string
Connection::Connection(SQLHENV hEnv_, SQLHDBC hDbc_, const wxString &str)
{
  hEnv = hEnv_;
  hDbc = hDbc_;
  
  connectString = str;

  GetTypeInfo();
}


// Destructor
Connection::~Connection()
{
  Close();
  typeInfoMap.clear();
}


// Close connection
void  Connection::Close()
{
  // Commit all pending Statements
  // Commit();
  SQLRETURN ret;

  if (hDbc != SQL_NULL_HANDLE)
  {
    if ( (ret = SQLDisconnect(hDbc)) == SQL_ERROR ||
         (ret = SQLFreeHandle(SQL_HANDLE_DBC, hDbc)) == SQL_ERROR)
    {
      throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, hDbc);
    }
  }

  hDbc = SQL_NULL_HANDLE;
}


// Query if connection is closed
bool  Connection::IsClosed() const
{
  return (hDbc == SQL_NULL_HANDLE);
}


// Check if the connection is still valid
bool  Connection::IsValid()
{
  try 
  {
    Statement *stmnt = CreateStatement();
    ResultSet *res = stmnt->ExecuteQuery("SELECT 1");

    delete res;
    delete stmnt;
  }
  catch (SQLException &)
  {
    return false;
  }

  return true;
}


// Create statement object
Statement * Connection::CreateStatement()
{
  SQLHSTMT  hStmt = SQL_NULL_HANDLE;
  SQLRETURN ret;

  if (!hDbc)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, NULL);

  // Allocate new handle
  ret = SQLAllocStmt(hDbc, &hStmt);
  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, hDbc);

  // Create new Statement object
  Statement  *stmtPtr = new Statement(hStmt, hDbc, hEnv);

  RegisterStatement(stmtPtr);  

  return stmtPtr;
}


// Prepare a statement
PreparedStatement * Connection::PrepareStatement(const wxString &str)
{
  SQLHSTMT  hStmt = SQL_NULL_HANDLE;
  SQLRETURN ret;

  wxASSERT(hDbc);

  ret = SQLAllocStmt(hDbc, &hStmt);
  if (ret == SQL_ERROR)
    throw SQLException(__TFILE__, __LINE__, SQL_NULL_HANDLE, hDbc);

  ret = SQLPrepare(hStmt, (SQLTCHAR *) str.wx_str(), SQL_NTS);
  if (ret == SQL_ERROR)
  {
    SQLException e(__TFILE__, __LINE__, hStmt, hDbc);
    SQLFreeStmt(hStmt, SQL_CLOSE);
    throw(e);
  }

  PreparedStatement *stmtPtr = new PreparedStatement(hStmt, hDbc, hEnv);

  RegisterStatement(stmtPtr);

  return stmtPtr;
}


// Set autocommit option
void  Connection::SetAutoCommit(bool enable)
{
  SQLSetConnectOption(hDbc, SQL_AUTOCOMMIT, 
     (enable ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF));
}


// Start Transaction. Transaction are not autocomitted
void  Connection::StartTransaction()
{
  SetAutoCommit(false);
}


// Commit / rollback transaction
void  Connection::Commit()
{
	SQLTransact (hEnv, hDbc, SQL_COMMIT);

  // Go back to default
  SetAutoCommit(true);
}


void  Connection::Rollback()
{
	SQLTransact (hEnv, hDbc, SQL_ROLLBACK);

  // Go back to default
  SetAutoCommit(true);
}


// -----------------------------------------------------------------------
// Typeinfo einlesen
bool  Connection::GetTypeInfo()
{
  if (hDbc == SQL_NULL_HANDLE)
    return false;

  SQLHSTMT  hStmt = SQL_NULL_HANDLE;
  SQLRETURN ret;

  unsigned short  bExists;

  typeInfoMap.clear();

  ret = SQLGetFunctions(hDbc, SQL_API_SQLGETTYPEINFO, &bExists);
  if (ret == SQL_ERROR || !bExists)
  {
    typeInfoMap.insert(TypeInfoMap::value_type(SQL_VARCHAR, "VARCHAR"));
    typeInfoMap.insert(TypeInfoMap::value_type(SQL_INTEGER, "INTEGER"));
    typeInfoMap.insert(TypeInfoMap::value_type(SQL_SMALLINT, "SMALLINT"));
    typeInfoMap.insert(TypeInfoMap::value_type(SQL_TIMESTAMP, "DATETIME"));
    
    return true;
  }

  if (SQLAllocStmt(hDbc, &hStmt) == SQL_ERROR)
    return false;

  if (SQLGetTypeInfo(hStmt, SQL_ALL_TYPES) == SQL_ERROR)
  {
    SQLFreeStmt(hStmt, SQL_DROP);
    return false;
  }

  wxChar  typeName[128];
  short   dataType;

  SQLLEN  typeNameNull;
  SQLLEN  dataTypeNull;

  SQLBindCol(hStmt, 1, SQL_C_TCHAR, typeName, 128, &typeNameNull);
  SQLBindCol(hStmt, 2, SQL_C_SSHORT, &dataType, 0, &dataTypeNull); 

  while (SQLFetch(hStmt) == SQL_SUCCESS)
  {
    typeInfoMap.insert(TypeInfoMap::value_type(dataType, typeName));
  }

  SQLFreeStmt(hStmt, SQL_DROP);

  // SQL_TIMESTAMP immer auf DATETIME.
  // MS SQL Server mappt auf DATETIME2, das ergibt aber Fehler z.B. im ReportManager

  typeInfoMap[SQL_TIMESTAMP] = "DATETIME";

  return true;
}


// Und Typ abfragen
wxString  Connection::GetDataType(short sqlType)
{
  TypeInfoMap::iterator  it = typeInfoMap.find(sqlType);
  if (it == typeInfoMap.end())
    return wxString("");
  else
    return (*it).second;
}


// -----------------------------------------------------------------------
void  Connection::RegisterStatement(Statement *stmtPtr)
{
  stmtVector.push_back(stmtPtr);
}


void  Connection::DeregisterStatement(Statement *stmtPtr)
{
  StmtPtrVector::iterator it = stmtVector.begin();
  while (it != stmtVector.end())
  {
    if (*it == stmtPtr)
    {
      stmtVector.erase(it);
      return;
    }
  }
}
