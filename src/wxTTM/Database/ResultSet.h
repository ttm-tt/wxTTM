/* Copyright (C) 2020 Christoph Theis */

// Implementation of a ResultSet

# ifndef  RESULTSET_H
# define  RESULTSET_H

#include  "SQLInclude.h"
#include  <string>
#include  <map>

#pragma warning(disable:4786)


class  ResultSet
{
  typedef  std::map<short, SQLLEN, std::less<short> >  NullDataMap;

  public:
    // Constructor, Destructor
    ResultSet(SQLHSTMT);
   ~ResultSet();

    // Get data
    bool  GetData(int nr, bool &data);
    bool  GetData(int nr, short &data);
    bool  GetData(int nr, long &data);
    bool  GetData(int nr, double &data);
    bool  GetData(int nr, wxChar * data, int len);
    bool  GetData(int nr, TIMESTAMP_STRUCT &data);
    bool  GetData(int nr, void * &data, size_t &len);

    // Abfrage, ob der letzt Zugriff NULL war
    bool  WasNull();

    // Bind columns
    bool  BindCol(int nr, bool *dataPtr);
    bool  BindCol(int nr, short *dataPtr);
    bool  BindCol(int nr, long *dataPtr);
    bool  BindCol(int nr, double *dataPtr);
    bool  BindCol(int nr, wxChar *dataPtr, int len);
    bool  BindCol(int nr, TIMESTAMP_STRUCT *dataPtr);
    
    // Abfrage, ob eine bestimme Spalte NULL war
    bool  WasNull(int nr);

    // Fetch Data
    bool  Next();

    // Get Column count
    int   GetColumnCount();

    // Get Column Label
    wxString GetColumnLabel(int nr);

  private:
    SQLHSTMT hStmt;
    NullDataMap  nullData;
    // unsigned long nullData[32];
};

# endif