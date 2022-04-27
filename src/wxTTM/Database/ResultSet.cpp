/* Copyright (C) 2020 Christoph Theis */

// Implementation of ResultSet object

# include  "stdafx.h"

# include  "ResultSet.h"
# include  "SQLException.h"


ResultSet::ResultSet(SQLHSTMT stmt)
{
  // memset(nullData, 0, sizeof(nullData));
  hStmt = stmt;
}


ResultSet::~ResultSet()
{
}


// -----------------------------------------------------------------------
// Get data
bool  ResultSet::GetData(int nr, bool &data)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_BIT, &data, 0, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, short &data)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_SSHORT, &data, 0, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, long &data)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_SLONG, &data, 0, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, double &data)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_DOUBLE, &data, 0, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, wxChar * data, int len)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_TCHAR, data, len, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, TIMESTAMP_STRUCT &data)
{
  SQLRETURN  ret;

  ret = SQLGetData(hStmt, nr, SQL_C_TIMESTAMP, &data, 0, &(nullData[0]));
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::GetData(int nr, void *&data, size_t &len)
{
  SQLRETURN ret;
  char tmp;  // SQLGetData darf keinen NULL-Pointer haben

  ret = SQLGetData(hStmt, nr, SQL_C_BINARY, &tmp, 0, &(nullData[0]));

  // SQL_SUCCESS bedeutet, dass es kein Bild gibt
  if (SQL_FAILED(ret))
    return false;

  if (ret == SQL_SUCCESS_WITH_INFO)
  {
    len = nullData[0];
    data = new char[len];
    ret = SQLGetData(hStmt, nr, SQL_C_BINARY, data, len, &nullData[0]);
  }
  
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);
  
  return true;
}


bool  ResultSet::WasNull()
{
  return nullData[0] == SQL_NULL_DATA;
}


// -----------------------------------------------------------------------
// Bind columns
bool  ResultSet::BindCol(int nr, bool *dataPtr)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_BIT, dataPtr, 0, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);


  return true;
}


bool  ResultSet::BindCol(int nr, short *dataPtr)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_SSHORT, dataPtr, 0, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::BindCol(int nr, long *dataPtr)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_SLONG, dataPtr, 0, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::BindCol(int nr, double *dataPtr)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_DOUBLE, dataPtr, 0, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::BindCol(int nr, wxChar * dataPtr, int len)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_TCHAR, dataPtr, len, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::BindCol(int nr, TIMESTAMP_STRUCT *dataPtr)
{
  SQLRETURN  ret;

  ret = SQLBindCol(hStmt, nr, SQL_C_TIMESTAMP, dataPtr, 0, &nullData[nr]);
  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  return true;
}


bool  ResultSet::WasNull(int nr)
{
  return nullData[nr] == SQL_NULL_DATA;
}

// -----------------------------------------------------------------------
// Fetch Data
bool  ResultSet::Next()
{
  if (hStmt == SQL_NULL_HSTMT)
    return false;

  SQLRETURN  ret;

  ret = SQLFetch(hStmt);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt, SQL_NULL_HDBC, SQL_NULL_HENV);

  if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    return true;

  if (ret == SQL_NO_DATA)
    return false;

  return false;
}


// -----------------------------------------------------------------------
// Get Column Count
int  ResultSet::GetColumnCount()
{
  SQLRETURN  ret;
  short      numOfCols;

  ret = SQLNumResultCols(hStmt, &numOfCols);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return numOfCols;
}


// Get Column Label
wxString  ResultSet::GetColumnLabel(int col)
{
  SQLRETURN  ret;
  wxChar  label[128];
  short   len;
  SQLLEN  info;

  ret = SQLColAttribute(hStmt, col, SQL_COLUMN_LABEL, label, 127, &len, &info);

  if (SQL_FAILED(ret))
    throw SQLException(__TFILE__, __LINE__, hStmt);

  return wxString(label);
}