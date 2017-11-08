#pragma once


// MinFindPropertyPage dialog

class MinFindPropertyPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(MinFindPropertyPage)

public:
	MinFindPropertyPage();
	virtual ~MinFindPropertyPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_ALGO };
#endif

protected:

	unsigned int SteepestDescentSteps;
	unsigned int MinFindSteps;
	
	int algorithm; // 0 - line minimization, 1 - preconditioned line minimization, 2 - Polak Ribiere conjugate gradient, 3 - Hestenes Stiefel conj grad, 4 - Fletcher Reeves conj grad

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnApply();

	void ApplyValues();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
};
