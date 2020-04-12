/* Copyright (C) 2020 Christoph Theis */

// Basisobjekt fuer Tables und Views
#ifndef  STOREOBJ_H
#define  STOREOBJ_H

#include  "TTDbse.h"

#include  "SQLInclude.h"
#include  "Statement.h"
#include  "ResultSet.h"

#include <string>
#include <set>


class  StoreObj
{
  public:
    static wxString  ltostr(const std::set<long> &list);
    static wxString  ltostr(long id);
    static wxString  tstostr(const timestamp &ts);
    static wxString  tstostr(const std::vector<timestamp> &list);
    
    static wxString  TransformString(const wxString &str);

  public:
    // Naechsten Record lesen
    virtual bool  Next();
    // Abfrage, ob eine Spalte NULL war
    bool  WasNull(int col);
    // Commit / Rollback
    void  Commit();
    void  Rollback();

    // Statementhandle freigeben
    void  Close();

    // Initialisierung vor dem naechsten Read
    virtual void Init() = 0;

    // Aktuelle Daten wurden gelesen
    bool  WasOK() const {return m_read;}

    // Liefert entweder m_conn oder TTDbse::defaultConnection
    Connection * GetConnectionPtr();

  protected:
    StoreObj(Connection *connPtr);
    StoreObj(StoreObj *refPtr = 0);
    virtual  ~StoreObj();

    StoreObj & operator=(const StoreObj &val);

    bool  ExecuteQuery(const wxString &sql);
    bool  ExecuteUpdate(const wxString &sql);

    bool  BindCol(int col, bool *dataPtr);
    bool  BindCol(int col, short *dataPtr);
    bool  BindCol(int col, long  *dataPtr);
    bool  BindCol(int col, double *dataPtr);
    bool  BindCol(int col, wxChar *dataPtr, int len);
    bool  BindCol(int col, TIMESTAMP_STRUCT *dataPtr);

    bool  GetData(int col, bool &dataPtr);
    bool  GetData(int col, short &dataPtr);
    bool  GetData(int col, long  &dataPtr);
    bool  GetData(int col, double &dataPtr);
    bool  GetData(int col, wxChar *dataPtr, int len);
    bool  GetData(int col, TIMESTAMP_STRUCT &dataPtr);

  private:
    Connection  *m_conn;
    Statement   *m_stmt;
    ResultSet   *m_res;

    bool         m_read;      // Data wurden aus DB gelesen
};


inline  StoreObj::StoreObj(Connection *connPtr)
{
  m_conn = connPtr;
  m_stmt = 0;
  m_res  = 0;
  m_read = false;
}


inline  StoreObj::StoreObj(StoreObj *refPtr)
{
  m_conn = (refPtr ? refPtr->m_conn : 0);
  m_stmt = 0;
  m_res  = 0;

  m_read = false;
}


inline  StoreObj::~StoreObj()
{
  delete m_stmt;
  delete m_res;
}


inline  StoreObj & StoreObj::operator=(const StoreObj &val)
{
  if (this == &val)
    return *this;

  m_conn = val.m_conn;
  m_stmt = 0;
  m_res  = 0;

  m_read = val.m_read;

  return *this;
}


inline  bool  StoreObj::BindCol(int col, bool *dataPtr)
{
  return (m_res ? m_res->BindCol(col, dataPtr) : false);
}


inline  bool  StoreObj::BindCol(int col, short *dataPtr)
{
  return (m_res ? m_res->BindCol(col, dataPtr) : false);
}


inline  bool  StoreObj::BindCol(int col, long *dataPtr)
{
  return (m_res ? m_res->BindCol(col, dataPtr) : false);
}


inline  bool  StoreObj::BindCol(int col, double *dataPtr)
{
  return (m_res ? m_res->BindCol(col, dataPtr) : false);
}


inline  bool  StoreObj::BindCol(int col, wxChar *dataPtr, int len)
{
  return (m_res ? m_res->BindCol(col, dataPtr, len) : false);
}


inline  bool  StoreObj::BindCol(int col, TIMESTAMP_STRUCT *dataPtr)
{
  return (m_res ? m_res->BindCol(col, dataPtr) : false);
}


inline  bool  StoreObj::GetData(int col, bool &data)
{
  return (m_res ? m_res->GetData(col, data) : false);
}


inline  bool  StoreObj::GetData(int col, short &data)
{
  return (m_res ? m_res->GetData(col, data) : false);
}


inline  bool  StoreObj::GetData(int col, long &data)
{
  return (m_res ? m_res->GetData(col, data) : false);
}


inline  bool  StoreObj::GetData(int col, double &data)
{
  return (m_res ? m_res->GetData(col, data) : false);
}


inline  bool  StoreObj::GetData(int col, wxChar *data, int len)
{
  return (m_res ? m_res->GetData(col, data, len) : false);
}


inline  bool  StoreObj::GetData(int col, TIMESTAMP_STRUCT &data)
{
  return (m_res ? m_res->GetData(col, data) : false);
}


inline  bool  StoreObj::WasNull(int col)
{
  return (m_res ? m_res->WasNull(col) : false);
}


inline  wxString  StoreObj::TransformString(const wxString &str)
{
  wxString  ret = str;
  ret.Replace("\'", "\'\'", true);

  return ret;
}


inline  wxString  StoreObj::ltostr(long id)
{
  char  buf[32];
  _ltoa(id, buf, 10);
  return wxString(buf);
}


inline wxString  StoreObj::ltostr(const std::set<long> &list)
{
  wxString ret;
  for (auto it = list.cbegin(); it != list.cend(); it++)
    ret += ltostr(*it) + ",";

  // Add dummy so we don't have to check for empty strings
  ret += "0";

  return ret;
}


inline  wxString StoreObj::tstostr(const timestamp &ts)
{
  wxChar  buf[32];
  wxSprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
            ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.fraction);

  return wxString(buf);
}

inline  wxString StoreObj::tstostr(const std::vector<timestamp> &list)
{
  wxString ret;

  // Add impossible date so we don't have to check for empty strings
  ret = "'1970-01-01T00:00:00'";

  for (const timestamp &ts : list)
    ret += ",'" + tstostr(ts) + "'";

  // FOR SYSTEM_TIME CONTAINING doesn't like 1-element arrays, so add a date in the future
  ret += ",'9999-12-31T23:59:59.999'";

  return ret;
}


#endif