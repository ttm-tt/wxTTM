/* Copyright (C) 2020 Christoph Theis */

// Datenbase engine for TTM

# ifndef  TTDBSE_H
# define  TTDBSE_H


#include  "SQLInclude.h"
#include  "Connection.h"

#include  <list>
#include  <string>

class  CpStore;
class  LtStore;
class  NaStore;


// Wrapper fuer die Connections etc.
class  TTDbse
{
  // Tabelle der verwendeten Typen
  // Singleton-Funktionen
  public:
    static TTDbse * instance() {return selfPtr ? selfPtr : (selfPtr = new TTDbse);}

 // Some helpers
    // Ist es eine lokale Adresse
    static bool  IsLocalAddress(const wxString &addr);

    // Connection string parsen
    static bool GetWindowsAuthentication(const wxString &connStr);
    static wxString GetServer(const wxString &connStr);
    static wxString GetDatabase(const wxString &connStr);
    static wxString GetUser(const wxString &connStr);
    static wxString GetPassword(const wxString &connStr);

  private:
    static  TTDbse * selfPtr;

  public:
    // Engine ist eine Verbindung zur Datenbank
    TTDbse() {defaultConnection = NULL;}
   ~TTDbse() {delete defaultConnection;};

  public:
    // Allgemeine API fuer Datenbank:
    // Datenbank oeffnen bzw. erzeugen
    bool CreateDatabase(
        const wxString &name, const wxString &server,
        bool windowsAuthentication, const wxString &user, const wxString &pwd, 
        short &type, short &table);
    bool OpenDatabase(const wxString &connStr, bool throwError = true);
    bool DetachDatabase(const wxString &connStr, bool throwError = true);
    bool CloseDatabase();
    
    bool BackupDatabase(const wxString &fileName);
    
    bool RestoreDatabase(const wxString &fileName, const wxString &dir);

    // Defaultconnection
    Connection * GetDefaultConnection();

    // Clone der Connection
    Connection * GetNewConnection();
    
    // Connect string
    const wxString & GetConnectionString() const {return strDsn;}

    wxString GetServer() const;
    wxString GetDatabase() const;
    bool     IsWindowsAuthenticaton() const;
    wxString GetUsername() const;
    wxString GetPassword() const;
    
    // List all servers in the network
    std::list<wxString> ListServers() const;
    
    // List all databases on server. Return a std::list<wxString>
    std::list<wxString> ListDatabases(const wxString &server, const wxString &connStr = wxEmptyString) const;

    // Test if tournament has schedule
    bool IsScheduled() const;

    // Test if tournament has started
    bool IsStarted() const;

    // Test if tournament has finished
    bool IsFinished() const;

  private:
    // Rollen fuer DB erzeugen
    bool  CreateRoles();
    // Domains fuer DB erzeugen
    bool  CreateDomains();
    // Neue Datenbank erzeugen
    bool  CreateTables();
    // Update einer Datenbank
    bool  UpdateTables(long version);

    wxString GetDsnPart(const wxString &part) const;

  private:
    wxString  strDsn;
    wxString  strUser;
    wxString  strPwd;
    
    Connection * defaultConnection;
};


inline  bool TTDbse::CloseDatabase()
{
  delete defaultConnection;
  defaultConnection = 0;

  return true;
}


# endif