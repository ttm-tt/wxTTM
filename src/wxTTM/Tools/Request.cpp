/* Copyright (C) 2020 Christoph Theis */

// Schnittstelle zwischen Datenbank und Views

#include "stdafx.h"

#include "Request.h"

CRequest::CRequest()
{
  type = NOTYPE;
  rec  = NOREC;
  id   = 0;
  tid = GetCurrentThread();
}


CRequest::CRequest(const CRequest &req)
    : type(req.type), rec(req.rec), id(req.id), tid(req.tid) 
{
}


CRequest::~CRequest()
{
}


