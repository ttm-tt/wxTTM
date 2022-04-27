/* Copyright (C) 2020 Christoph Theis */

// Implementation of a SQL statement

# ifndef  STATEMENT_H
# define  STATEMENT_H

#include  <windows.h>
#include  <sql.h>
#include  <sqlext.h>

#include  <string>


class  ResultSet;


class  Statement
{
  public:
    // Constructor
    Statement(SQLHSTMT hStmt, SQLHDBC hDbc, SQLHENV hEnv);
    virtual ~Statement();

    // Execute a query, returns ResultSet
    ResultSet * ExecuteQuery(const wxString &query);  // throws SQLException

    // Execute an update, returns no. of affected rows
    int  ExecuteUpdate(const wxString &update);       // throws SQLException

    // Execute a statement. Returns true, if first 
    // result is a result set, false otherweise
    bool Execute(const wxString &sql);                // throws SQLException

    // close statement handle
    void  Close();

  private:
    // Copy and Assignment operator
    Statement(const Statement &) {}
    Statement & operator=(const Statement &) {}

  public:
    // Reset internals before reusing
    void  Reset();                                       // throws SQLException
    // Get number of columns in result set
    int   GetColumnCount();                              // throws SQLException
    // Get result set. Check for dup. calls
    ResultSet * GetResultSet(bool check); 

  protected:
    SQLHSTMT  hStmt = SQL_NULL_HSTMT;
    SQLHDBC   hDbc = SQL_NULL_HDBC;
    SQLHENV   hEnv = SQL_NULL_HENV;
};


class  PreparedStatement : public Statement
{
  public:
    // Constructor
    PreparedStatement(SQLHSTMT hStmt, SQLHDBC hDbc, SQLHENV hEnv);
    virtual ~PreparedStatement();

  public:
    bool  SetData(int nr, bool *dataPtr);
    bool  SetData(int nr, short *dataPtr);
    bool  SetData(int nr, long *dataPtr);
    bool  SetData(int nr, double *dataPtr);
    bool  SetData(int nr, wxChar *dataPtr);
    bool  SetData(int nr, timestamp *dataPtr);    
    bool  SetData(int nr, void * data, size_t len);

  public:
    bool  Execute();  // throws SQLException

  private:
    // TODO: Eine Queue dafuer
    SQLLEN  nullData[64] = {0};
};


# endif