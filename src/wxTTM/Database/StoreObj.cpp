/* Copyright (C) 2020 Christoph Theis */

#include  "stdafx.h"

#include  "StoreObj.h"

#include  "TT32App.h"
#include  "SQLException.h"
#include  "InfoSystem.h"



Connection * StoreObj::GetConnectionPtr()
{
  Connection *connPtr = 
    (m_conn ? m_conn : TTDbse::instance()->GetDefaultConnection());

  if (!connPtr)
  {
    throw SQLException(__TFILE__, __LINE__, _("Tournament not opend"));
  }

  return connPtr;
}


bool  StoreObj::ExecuteQuery(const wxString &sql)
{
  Connection *connPtr = GetConnectionPtr();

  if (m_res)
    delete m_res;
  if (m_stmt)
    delete m_stmt;

  m_stmt = 0;
  m_res = 0;

  m_stmt = connPtr->CreateStatement();

  m_res = m_stmt->ExecuteQuery(sql);
 
  if (!m_res)
    return false;

  return true;      
}


bool  StoreObj::ExecuteUpdate(const wxString &sql)
{
  Connection *connPtr = GetConnectionPtr();

  if (m_res)
    delete m_res;
  if (m_stmt)
    delete m_stmt;

  m_stmt = 0;
  m_res = 0;

  m_stmt = connPtr->CreateStatement();

  m_stmt->ExecuteUpdate(sql);

  return true;      
}



void  StoreObj::Commit()
{
  Connection *connPtr = GetConnectionPtr();

  try
  {
    connPtr->Commit();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception("Commit", e);
  }
}


void  StoreObj::Rollback()
{
  Connection *connPtr = GetConnectionPtr();

  try
  {
    connPtr->Rollback();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception("Rollback", e);
  }
}


void  StoreObj::Close()
{
  try
  {
    if (m_res)
      delete m_res;
    m_res = 0;

    if (m_stmt)
      delete m_stmt;
    m_stmt = 0;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception("Close", e);
  }
}


bool  StoreObj::Next()
{
  Init();

  if (!m_res)
    return (m_read = false);

  try
  {
    m_read = m_res->Next();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception("Next", e);
    m_read = false;
  }

  if (m_res && !m_read)
    Close();


  return m_read;
}


