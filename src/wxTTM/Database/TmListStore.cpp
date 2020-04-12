/* Copyright (C) 2020 Christoph Theis */

// View auf TmRec

#include  "stdafx.h"
#include  "TmListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "CpStore.h"
#include  "LtStore.h"
#include  "PlStore.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  TmListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW TmList AS "
                     "SELECT tm.tmID, cpID, tmDisqu, tmGaveup, tmName, tmDesc, na.naID "
                     "FROM TmRec tm LEFT OUTER JOIN RkRec rk ON tm.tmID = rk.tmID "
                     "              LEFT OUTER JOIN NaRec na ON rk.naID = na.naID ";

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


bool  TmListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW TmList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
TmListStore::TmListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


TmListStore::~TmListStore()
{
}


void  TmListStore::Init()
{
  TmListRec::Init();
}


// -----------------------------------------------------------------------
bool  TmListStore::SelectByCp(const CpRec &cp)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  TmListStore::SelectByCpPl(const CpRec &cp, const PlRec &pl, const timestamp *when)
{
  wxString  sel;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    sel = "SELECT ";
  }
  else
  {
    sel = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  wxString str = SelectString(when);
  str += " WHERE cpID = " + ltostr(cp.cpID) + " AND tmID IN "
         " (" + sel + " tmID FROM NtRec " + ts + "nt INNER JOIN LtRec " + ts + " lt ON nt.ltID = lt.ltID "
         "   INNER JOIN PlRec " + ts + " pl ON lt.plID = pl.plID "
         "   WHERE pl.plID = " + ltostr(pl.plID) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  TmListStore::SelectByLt(const LtRec &lt, const timestamp *when)
{
  wxString  sel;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    sel = "SELECT ";
  }
  else
  {
    sel = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  wxString str = SelectString(when);

  str += " WHERE tmID IN "
         " (" + sel + " tmID FROM NtRec " + ts + " nt INNER JOIN LtRec " + ts + " lt ON nt.ltID = lt.ltID "
         "   WHERE lt.ltID = " + ltostr(lt.ltID) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool TmListStore::SelectByPl(const PlRec &pl)
{
  wxString str = SelectString();
  str += " WHERE tmID IN "
         " (SELECT tmID FROM NtRec nt INNER JOIN LtRec lt ON nt.ltID = lt.ltID "
         "   INNER JOIN PlRec pl ON lt.plID = pl.plID "
         "   WHERE pl.plID = " + ltostr(pl.plID) + ")";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  TmListStore::SelectByName(const wxString &name, const CpRec &cp)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + " AND tmName = '" + TransformString(name) + "' ";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}



// -----------------------------------------------------------------------
wxString  TmListStore::SelectString(const timestamp *when) const
{
  wxString  str;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    str = "SELECT ";
  }
  else
  {
    str = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  str +=  " tmID, cpID, tmDisqu, tmGaveup, tmName, tmDesc, naID FROM TmList " + ts + " ";

  return str;
}


bool  TmListStore::BindRec()
{
  BindCol(1, &tmID);
  BindCol(2, &cpID);
  BindCol(3, &tmDisqu);
  BindCol(4, &tmGaveup);
  BindCol(5, tmName, sizeof(tmName));
  BindCol(6, tmDesc, sizeof(tmDesc));
  BindCol(7, &naID);

  return true;
}
