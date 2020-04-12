/* Copyright (C) 2020 Christoph Theis */

// Implementation of Connection interface class

#ifndef   CONNECTION_H
#define   CONNECTION_H


#include  <windows.h>
#include  <sql.h>
#include  <sqlext.h>

#include  <string>
#include  <vector>
#include  <map>

class  Statement;
class  PreparedStatement;

#pragma warning(disable:4786)

class  Connection
{
  typedef  std::map<short, wxString, std::less<short> >  TypeInfoMap;
  typedef  std::vector<Statement *>  StmtPtrVector;

  // public interface
  public:
    // Constructor, opens connection
    Connection(SQLHENV hEnv_, SQLHDBC hDbc_, const wxString &str);    // throws SQLException

   ~Connection();

    // Close connection, but retain object
    void  Close();                  // throws SQLException

    // Returns true if connection was closed
    bool  IsClosed() const; 

    // Test if the connection is still valid
    bool  IsValid();

    // Create a statement object 
    Statement * CreateStatement();  // throws SQLException
    // Prepare a statement
    PreparedStatement * PrepareStatement(const wxString &sql); // throws SQLException

    // Create a prepared statement object
    // PreparedStatement * prepareStatement(const char *sql); // throws SQLException

    // create a callable statement object for calling stored procedures
    // CallableStatement * prepareCall(const char * sql);     // throws SQLException;

    // setAutoCommit option
    void  SetAutoCommit(bool enable);

    // Start batch (transaction)
    void  StartTransaction();

    // Commit transaction
    void  Commit();                        // throws SQLException

    // Rollback transaction
    void  Rollback();                      // throws SQLException

    // Typeinfo abfragen
    wxString  GetDataType(short sqlType);

    void  DeregisterStatement(Statement *stmtPtr);
    
    const wxString GetConnectionString() const {return connectString;}
  
  // Variables
  protected:
    void  RegisterStatement(Statement *stmtPtr);

    // SQL handles
    SQLHENV   hEnv;  
    SQLHDBC   hDbc;

    // Connect string
    wxString  connectString;

  private:
    // Typeinfo einlesen
    bool  GetTypeInfo();
    TypeInfoMap  typeInfoMap;

    // Liste der Statements
    StmtPtrVector  stmtVector;
};


# endif
