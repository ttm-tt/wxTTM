/* Copyright (C) 2020 Christoph Theis */

// Relation Gr <--> ST / GR
#ifndef  XXSTORE_H
#define  XXSTORE_H

#include  "StoreObj.h"

class  Connection;

struct StRec;
struct GrRec;

struct  XxRec
{
  long  stID;  // To ST
  long  grID;  // From GR
  short grPos; // Evtl. from position 

  XxRec() {Init();}
  
  void Init() {memset(this, 0, sizeof(XxRec));}
};


class  XxStore : public StoreObj, public XxRec
{
  public:
    static  bool CreateTable();
    static  bool UpdateTable(long version);
    static  bool CreateConstraints();
    static  bool UpdateConstraints(long version);

  public:
    XxStore(Connection * = 0);
   ~XxStore();

    virtual void Init();

  public:
    bool Insert(const StRec &, const GrRec &, short pos = 1);
    bool Remove(const StRec &);
    bool Remove(const GrRec &, short pos = 0);  // pos == 0: All

    bool Select(const StRec &);
    bool Select(const GrRec &, short pos = 1);
    bool SelectAll();

  private:
    wxString  SelectString() const;
    bool  BindRec();

};


#endif