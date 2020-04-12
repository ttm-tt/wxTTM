/* Copyright (C) 2020 Christoph Theis */

// Implementation of the DriverManager

# ifndef  DRIVERMANAGER_H
# define  DRIVERMANAGER_H


#include  "SQLInclude.h"

#include  <string>

class  Connection;


class  DriverManager
{
  public:
    // DriverManager is Singleton
    static  DriverManager * instance() 
        {return selfPtr ? selfPtr : (selfPtr = new DriverManager);}

    DriverManager();
   ~DriverManager();

    // Create connection to database
    Connection * GetConnection(
        const wxString &dsn, 
        const wxString &uid, 
        const wxString &pwd);

    // Create connection to database with connection string
    Connection * GetConnection(const wxString &connStr);

    // Enumerate Datasource
    bool  GetFirstDataSource(wxString &strDbse, wxString &desc);
    bool  GetNextDataSource(wxString &strDbse, wxString &desc);

  protected:
    SQLHENV  hEnv;

  private:
    static  DriverManager * selfPtr;

};

# endif