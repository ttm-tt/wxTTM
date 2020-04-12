/* Copyright (C) 2020 Christoph Theis */

// Liste der "Mannschafts"meldungen
#ifndef  NTENTRYLIST_H
#define  NTENTRYLIST_H

#include "StoreObj.h"
#include "LtEntryStore.h"
#include "NtStore.h"


struct NtEntry : public LtEntry
{
  public:
    NtRec  nt;
    
  public:
    NtEntry() {memset(this, 0, sizeof(NtEntry));}
    NtEntry(const NtEntry &nt) {memcpy(this, &nt, sizeof(NtEntry));}
    
    void Init() {memset(this, 0, sizeof(NtEntry));}
};




class  NtEntryStore : public StoreObj, public NtEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();
    
  public:
    NtEntryStore(Connection *connPtr = 0);
    
    virtual void Init();
    virtual bool Next();
    
  public:
    bool  SelectByTm(const TmRec &tm);
    bool  SelectByLt(const LtRec &lt);
    
  private:
    wxString  SelectString() const;
    bool  BindRec();    
};

#endif