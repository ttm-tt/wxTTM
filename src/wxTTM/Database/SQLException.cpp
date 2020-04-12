/* Copyright (C) 2020 Christoph Theis */

// Implementation of SQLException

# include  "stdafx.h"

# include  "SQLException.h"


// Constructor
SQLException::SQLException(const wxString &file, long line,
    SQLHSTMT hStmt, SQLHDBC hDbc, SQLHENV hEnv)
{
  fileName = file;
  lineNr = line;

  // get error info
  SQLSMALLINT  textLength;
  SQLRETURN    ret;

  ret = SQLError( 
      hEnv, hDbc, hStmt,
	    (SQLTCHAR *) sqlState,
	    &errorCode,
	    (SQLTCHAR *) message, SQL_MAX_MESSAGE_LENGTH-1,
	    &textLength
  );
}


SQLException::SQLException(const wxString &file, long line, const wxString &msg)
{
  fileName = file;
  lineNr = line;
  wxStrncpy(message, msg, SQL_MAX_MESSAGE_LENGTH-1);
  message[SQL_MAX_MESSAGE_LENGTH-1] = '\0';
  wxStrcpy(sqlState, "");
  errorCode = 0;
}
