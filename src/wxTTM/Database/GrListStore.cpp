/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "GrListStore.h"
#include  "GrStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "CpStore.h"
#include  "TmStore.h"

#include  <stdio.h>
#include  <stdlib.h>

#include  <list>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  GrListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW GrList AS "
                     "SELECT grID, grName, grDesc, grStage, grModus, grSize, "
                     "cpID, grWinner, mdID, syID, grBestOf, grQualRounds, "
                     "grNofRounds, grNofMatches, grNoThirdPlace, grOnlyThirdPlace, "
                     "grPublished, grNotes, grSortOrder, CASE WHEN grNotes IS NULL THEN 0 ELSE 1 END AS grHasNotes,  "
                     "grPrinted "
                     "FROM GrRec";

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


bool  GrListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW GrList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
GrListStore::GrListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


GrListStore::~GrListStore()
{
}


void  GrListStore::Init()
{
  GrListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  GrListStore::SelectAll(const CpRec &cp)
{
  wxString  str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + " ORDER BY grSortOrder, grStage, grName";

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


bool  GrListStore::SelectById(long id)
{
  wxString  str = SelectString();
  str += "WHERE grID = " + ltostr(id);

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


bool  GrListStore::SelectById(const std::set<long> &idList)
{
  wxString  str = SelectString();
  str += "WHERE grID IN (" + ltostr(idList) + ")";

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


bool GrListStore::SelectByCpTm(const CpRec &cp, const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + 
         "   AND grID IN (SELECT grID FROM StRec "
         "                 WHERE tmID = " + ltostr(tm.tmID) + ")";

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


bool GrListStore::SelectByTm(const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE grID IN (SELECT grID FROM StRec "
         "                 WHERE tmID = " + ltostr(tm.tmID) + ")";

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


bool  GrListStore::SelectByStage(const CpRec &cp, const wxString &stage)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + 
         "   AND grStage = '" + TransformString(stage) + "'" +
         " ORDER BY grName";

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
std::list<wxString> GrListStore::ListStages(const CpRec &cp)
{
  wxString str = "SELECT DISTINCT grStage FROM GrList";
  
  if (cp.cpID != 0)
    str += " WHERE cpID = " + ltostr(cp.cpID);
    
  str += " ORDER BY 1";

  std::list<wxString> res;

  wxChar stage[65];

  try
  {
    if (!ExecuteQuery(str))
      return res;

    BindCol(1, stage, sizeof(stage));

    while (Next())
    {
      if (WasNull(1))
        continue;

      res.push_back(wxString(stage));
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return res;
}

// -----------------------------------------------------------------------
bool  GrListStore::QryStarted() 
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryStarted();
}


bool  GrListStore::QryFinished()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryFinished();
}


bool  GrListStore::QryChecked()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;
  
  return gr.QryChecked();
}


bool  GrListStore::QryCombined()
{
  GrStore gr(GetConnectionPtr());
  gr.grID = grID;

  return gr.QryCombined();
}

// -----------------------------------------------------------------------
long GrListStore::Count()
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = "SELECT COUNT(*) FROM GrList";

  long count = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();

    ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
    if (!resPtr || !resPtr->Next())
      count = 0;
    else if (!resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    count = 0;
  }

  delete resPtr;
  delete stmtPtr;

  return count;
}


wxString GrListStore::GetNote()
{
  return GrStore(*this, GetConnectionPtr()).GetNote();
}

// -----------------------------------------------------------------------
wxString  GrListStore::SelectString() const
{
  wxString  str = 
    "SELECT grID, grName, grDesc, grStage, grModus, grSize, grWinner, cpID, mdID, syID, grBestOf, "
    "       grQualRounds, grNofRounds, grNofMatches, grNoThirdPlace, grOnlyThirdPlace, "
    "       grPublished, grHasNotes, grSortOrder, grPrinted "
    "  FROM GrList "
    ;

  return str;
}


bool  GrListStore::BindRec()
{
  BindCol(1, &grID);
  BindCol(2, grName, sizeof(grName));
  BindCol(3, grDesc, sizeof(grDesc));
  BindCol(4, grStage, sizeof(grStage));
  BindCol(5, &grModus);
  BindCol(6, &grSize);
  BindCol(7, &grWinner);
  BindCol(8, &cpID);
  BindCol(9, &mdID);
  BindCol(10, &syID);
  BindCol(11, &grBestOf);
  BindCol(12, &grQualRounds);
  BindCol(13, &grNofRounds);
  BindCol(14, &grNofMatches);
  BindCol(15, &grNoThirdPlace);
  BindCol(16, &grOnlyThirdPlace);
  BindCol(17, &grPublished);
  BindCol(18, &grHasNotes);
  BindCol(19, &grSortOrder);
  BindCol(20, &grPrinted);

  return true;
}