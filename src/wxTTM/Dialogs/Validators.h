/* Copyright (C) 2020 Christoph Theis */

#ifndef _VALIDATORS_H_
#define _VALIDATORS_H_ 

class CShortValidator : public wxValidator
{
  public:
    CShortValidator();
    CShortValidator(short *val, bool suppressNull = false);
    CShortValidator(const CShortValidator &);
    
  public:
    wxObject * Clone() const;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    void OnChar(wxKeyEvent &);
    
  private:
    short *m_val;
    bool   m_suppressNull;
    
  DECLARE_DYNAMIC_CLASS(CShortValidator)
  DECLARE_EVENT_TABLE()
};


class CLongValidator : public wxValidator
{
  public:
    CLongValidator();
    CLongValidator(long *val, bool suppressNull = false);
    CLongValidator(const CLongValidator &);
    
  public:
    wxObject * Clone() const;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    void OnChar(wxKeyEvent &);
    
  private:
    long *m_val;
    bool  m_suppressNull;
    
  DECLARE_DYNAMIC_CLASS(CLongValidator)
  DECLARE_EVENT_TABLE()
};


class CDoubleValidator : public wxValidator
{
  public:
    CDoubleValidator();
    CDoubleValidator(double *val, const wxString &fmt = wxT("%lf"));
    CDoubleValidator(const CDoubleValidator &);
    
  public:
    wxObject * Clone() const;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    void OnChar(wxKeyEvent &);
    
  private:
    double *m_val;
    wxString m_fmt;
    
  DECLARE_DYNAMIC_CLASS(CDoubleValidator)
  DECLARE_EVENT_TABLE()
};


class CCharArrayValidator : public wxValidator
{
  public:
    CCharArrayValidator();
    CCharArrayValidator(wxChar *val, size_t len);
    CCharArrayValidator(const CCharArrayValidator &);
    
  public:
    wxObject * Clone() const;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    void OnChar(wxKeyEvent &);
    
  private:
    wxChar *m_val;
    size_t  m_len;
    
  DECLARE_DYNAMIC_CLASS(CCharArrayValidator)
  DECLARE_EVENT_TABLE()
};


class CEnumValidator : public wxValidator
{
  public:
    CEnumValidator();
    CEnumValidator(short *val, short set);
    CEnumValidator(const CEnumValidator &);
    
  public:
    wxObject * Clone() const;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    short *m_val;
    short  m_set;
    
  DECLARE_DYNAMIC_CLASS(CEnumValidator)
};


class CDateValidator : public wxValidator
{
  public:
    CDateValidator();
    CDateValidator(timestamp *val);
    CDateValidator(const CDateValidator &);
  
  public:
    wxObject * Clone() const ;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    timestamp *m_val;
    
  DECLARE_DYNAMIC_CLASS(CDateValidator)
};

class CTimeValidator : public wxValidator
{
  public:
    CTimeValidator(bool empty = true);
    CTimeValidator(timestamp *val, bool empty = true);
    CTimeValidator(const CTimeValidator &);
  
  public:
    wxObject * Clone() const ;
    
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow *parent);
    
  private:
    timestamp *m_val;
    bool emptyIfNull;

  DECLARE_DYNAMIC_CLASS(CTimeValidator)
};

class CDateTimeValidator : public wxValidator
{
public:
  CDateTimeValidator();
  CDateTimeValidator(timestamp* val);
  CDateTimeValidator(const CDateTimeValidator&);

public:
  wxObject* Clone() const;

  bool TransferFromWindow();
  bool TransferToWindow();
  bool Validate(wxWindow* parent);

private:
  timestamp* m_val;

  DECLARE_DYNAMIC_CLASS(CDateTimeValidator)
};

#endif