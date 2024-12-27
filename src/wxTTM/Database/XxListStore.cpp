/* Copyright (C) 2020 Christoph Theis */

// View auf Verknuepfung zwischen Gruppen

#include  "stdafx.h"
#include  "XxListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  XxListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW XxList (stID, grID, grPos, grName, grDesc) "
                     "AS SELECT xx.stID, xx.grID, xx.grPos, gr.grName, gr.grDesc "
                     "FROM XxRec xx LEFT OUTER JOIN GrRec gr ON xx.grID = gr.grID";

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


bool  XxListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS XxList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


