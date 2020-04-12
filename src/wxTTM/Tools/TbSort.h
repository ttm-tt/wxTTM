/* Copyright (C) 2020 Christoph Theis */

// Tabellenberechnung und Sortieren

#ifndef  TBSORT_H
#define  TBSORT_H

#include  <vector>
#include  <functional>

struct  GrRec;
struct  MtRec;
class   TbItem;


class  TbSort
{
  public:
    static  bool  Sort(const GrRec &gr, short cpType, 
                       std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);

  protected:
    struct  CompareResultFunctional
    {
      bool  operator()(TbItem *&i1, TbItem *&i2) 
      { return CompareResult && ( (*CompareResult)(i1, i2) < 0) ? true : false; }
    }; 

    static int  MaxMatchPoints(TbItem *, TbItem *);
    static int  MaxPoints(TbItem *, TbItem *);
    static int  CompPoints(TbItem *, TbItem *);
    static int  CompMatches(TbItem *, TbItem *);
    static int  CompSets(TbItem *, TbItem *);
	  static int  CompBalls(TbItem *, TbItem *);

	  static  void CompDiff() {Comp = Diff;}
	  static  void CompQuot() {Comp = Quot;}

	  // Zwei Vergleichstypen: Differenz und Quotient
	  static  int  Diff(const short *,const short *);    // Vergleicht mit Diff.
	  static  int  Quot(const short *,const short *);    // Vergleicht mit Quot.

   public:
     // Vergleich nach Punkten, Saetzen, Baellen, ...
     static  int  (*CompareResult)(TbItem *, TbItem *);

   private:
     // Vergleich von Differenzen / Quotienten
	   static  int  (*Comp)(const short *,const short *);

  protected:
    virtual bool  SumUp(short from, short to, const GrRec &rec, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList) = 0;

    virtual bool  DoSort(short from, short to, const GrRec &gr, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList) = 0;
    virtual bool  DirectComp(short from, short to, const GrRec &gr, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList) = 0;

  protected:
    bool  SortRange(short from, short to, std::vector<TbItem *> &tbList, 
                int (*)(TbItem *, TbItem*));

    bool  NumberRange(short from, short to, std::vector<TbItem *> &tbList, 
                int (*)(TbItem *, TbItem*));
};


class  IttfSort : public TbSort
{
  friend class TbSort;

  public:
    IttfSort() {};

  protected:
    virtual bool  SumUp(short from, short to, const GrRec &rec, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);

    virtual bool  DoSort(short from, short to, const GrRec &gr, short cpType,  
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);

    virtual bool  DirectComp(short from, short to, const GrRec &gr, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);
};


class  DttbSort : public TbSort
{
  friend class TbSort;

  public:
    DttbSort() {};
    
  protected:
    virtual bool  SumUp(short from, short to, const GrRec &rec, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);

    virtual bool  DoSort(short from, short to, const GrRec &gr, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);
    
    virtual bool  DirectComp(short from, short to, const GrRec &gr, short cpType,
                         std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList);

};

#endif