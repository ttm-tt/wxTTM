/* Copyright (C) 2020 Christoph Theis */

// Implementation of statement object

#include  "stdafx.h"

#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"


// Constructor, Destructor
Statement::Statement(SQLHSTMT hStmt_, SQLHDBC hDbc_, SQLHENV hEnv_)
{
  hStmt = hStmt_;
  hDbc  = hDbc_;
  hEnv  = hEnv_;
}


Statement::~Statement()
{
  Close();
}


// Execute query
ResultSet * Statement::ExecuteQuery(const wxString &sql)
{
  ResultSet  *rsPtr = NULL;

  if (Execute(sql))
    rsPtr = GetResultSet(false);

  // TODO: Fehlermeldung, wenn execute keinen RS lieferte
  return rsPtr;
}


// Execute update
int  Statement::ExecuteUpdate(const wxString &sql)
{
  int  numOfCols = -1;

  if (!Execute(sql))
    numOfCols = GetColumnCount();

  // TODO: Fehlerbehandlung, wenn Execute true liefert, 
  // also ein ResultSet erzeugt wurde

  return numOfCols;
}


// Execute statement
bool Statement::Execute(const wxString &sql)
{
  SQLRETURN  ret;
  bool  hasResultSet = false;

  // Reset stmt handle
  Reset();

  // lock if necessary (FOR UPDATE statement)
  // LockIfNecessary(sql);

  // Execute statement
  ret = SQLExecDirect(hStmt, (SQLTCHAR *) sql.wx_str(), SQL_NTS);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  if (GetColumnCount() > 0)
    hasResultSet = true;

  return hasResultSet;
}


// Reset internal state
void Statement::Reset()
{
  if (hStmt != SQL_NULL_HSTMT)
  {
    SQLFreeStmt(hStmt, SQL_CLOSE);
  }

  // handle remains valid
}


// Close statement
void  Statement::Close()
{
  SQLRETURN  ret;
  if (hStmt != SQL_NULL_HSTMT)
  {
    ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    hStmt = SQL_NULL_HSTMT;

    if (SQL_FAILED(ret))
      throw SQLException(__TFILE__, __LINE__, SQL_NULL_HSTMT, hDbc);
  }
}


// Get number of columns in result set
int  Statement::GetColumnCount()
{
  SQLRETURN  ret;
  short      numOfCols;

  ret = SQLNumResultCols(hStmt, &numOfCols);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return numOfCols;
}


// Get result set of last query
ResultSet * Statement::GetResultSet(bool checkCount)
{
  ResultSet * rsPtr = NULL;

  // TODO: checkCount testet, ob RS bereits geliefert wurde.

  rsPtr = new ResultSet(hStmt);
  return rsPtr;
}


// -----------------------------------------------------------------------
// PreparedStatement
// Constructor, Destructor
PreparedStatement::PreparedStatement(SQLHSTMT hStmt_, SQLHDBC hDbc_, SQLHENV hEnv_)
                 : Statement(hStmt_, hDbc_, hEnv_)
{
  memset(nullData, 0, sizeof(nullData));
}


PreparedStatement::~PreparedStatement()
{
}


// Bind Parameter
bool  PreparedStatement::SetData(int nr, bool *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = 0;

  ret = SQLBindParameter(hStmt, nr, SQL_PARAM_INPUT, SQL_C_BIT, SQL_SMALLINT, 
                         0, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, short *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = 0;

  ret = SQLBindParameter(hStmt, nr, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 
                         0, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, long *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = 0;

  ret = SQLBindParameter(hStmt, nr, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 
                         0, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, double *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = 0;

  ret = SQLBindParameter(hStmt, nr, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 
                         0, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, wxChar *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = SQL_NTS;

  ret = SQLBindParameter(hStmt, nr, SQL_PARAM_INPUT, SQL_C_TCHAR, SQL_TVARCHAR, 
                         SQL_NTS, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, timestamp *dataPtr)
{
  SQLRETURN  ret;

  if (!dataPtr)
    nullData[nr] = SQL_NULL_DATA;
  else
    nullData[nr] = SQL_NTS;

  ret = SQLBindParameter(
        hStmt, nr, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, 
        0, 0, dataPtr, 0, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}


bool  PreparedStatement::SetData(int nr, void *data, size_t len)
{
  SQLRETURN ret;
  
  ret = SQLBindParameter(
        hStmt, nr, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, 
        len, 0, data, 1, &nullData[nr]);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);
    
  return true;
}


// Execute prepared statement
bool  PreparedStatement::Execute()
{
  SQLRETURN  ret;

  wxASSERT(hStmt);

  ret = SQLExecute(hStmt);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return true;
}