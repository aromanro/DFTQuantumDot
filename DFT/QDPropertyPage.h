#pragma once


#include "NumberEdit.h"

// QDPropertyPage dialog

class QDPropertyPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(QDPropertyPage)

public:
	QDPropertyPage();
	virtual ~QDPropertyPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_QD };
#endif

protected:
	double dim1;
	double dim2;
	double dim3;

	CNumberEdit dim1Edit;
	CNumberEdit dim2Edit;
	CNumberEdit dim3Edit;

	int samples1;
	int samples2;
	int samples3;

	int Ns;

	int harmonic; // 1 - harmonic potential, 0 - linear

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	void ApplyValues();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnBnClickedRadio();
};
