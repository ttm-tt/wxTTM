/* Copyright (C) Christoph Theis 2022 */

#include "stdafx.h"
#include "MpListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"


// -----------------------------------------------------------------------
bool  MpListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW MpList AS "
                     "SELECT mpID, mpName, mpDesc, mpCount "
                     "  FROM MpRec";

  try
  {
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  MpListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW MpList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
MpListStore::MpListStore(Connection* connPtr) : StoreObj(connPtr)
{

}



MpListStore::~MpListStore()
{

}


void MpListStore::Init()
{
  MpListRec::Init();
}


// -----------------------------------------------------------------------
bool  MpListStore::SelectAll()
{
  wxString stmt = SelectString();
  stmt += " ORDER BY mpName";

  try
  {
    if (!ExecuteQuery(stmt))
      return false;
      
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


bool  MpListStore::SelectById(long id)
{
  wxString stmt = SelectString();
  stmt += " WHERE mpID = ";
  stmt += ltostr(id);

  try
  {
    if (!ExecuteQuery(stmt))
      return false;
      
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString MpListStore::SelectString() const
{
  return 
    "SELECT mpID, mpName, mpDesc, mpCount FROM MpList ";
}


bool MpListStore::BindRec()
{
  BindCol(1, &mpID);
  BindCol(2, mpName, sizeof(mpName));
  BindCol(3, mpDesc, sizeof(mpDesc));
  BindCol(4, &mpCount);

  return true;
}