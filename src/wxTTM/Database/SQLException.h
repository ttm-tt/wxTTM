/* Copyright (C) 2020 Christoph Theis */

// Implementation of SQLException

# ifndef  SQLEXCEPTION_H
# define  SQLEXCEPTION_H

#include "SQLInclude.h"

class  SQLException
{
  public:
    SQLException(
      const wxString &file, long line,
      SQLHSTMT hStmt = SQL_NULL_HANDLE, 
      SQLHDBC  hDbc  = SQL_NULL_HANDLE,
      SQLHENV  hEnv  = SQL_NULL_HANDLE);

    SQLException(
      const wxString &file, long line, const wxString &msg);

    const wxString &GetFileName() const  {return fileName;}
    long  GetLineNr() const           {return lineNr;}
    wxString GetMessage()  const      {return wxString(message);}
    wxString GetSQLState() const      {return wxString(sqlState);}
    long  GetErrorCode()  const       {return errorCode;}    

  protected:
    wxString fileName;
    long   lineNr;
    wxChar message[SQL_MAX_MESSAGE_LENGTH];
    wxChar sqlState[20];
    long   errorCode;

};


# endif
