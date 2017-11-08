// MinFindPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "MinFindPropertyPage.h"
#include "afxdialogex.h"
#include "resource.h"

#include "DFT.h"


// MinFindPropertyPage dialog

IMPLEMENT_DYNAMIC(MinFindPropertyPage, CMFCPropertyPage)

MinFindPropertyPage::MinFindPropertyPage()
	: CMFCPropertyPage(IDD_PROPPAGE_ALGO)
{
	SteepestDescentSteps = theApp.options.SteepestDescentSteps;
	MinFindSteps = theApp.options.MinFindSteps;
	algorithm = theApp.options.algorithm;
}

MinFindPropertyPage::~MinFindPropertyPage()
{
}

void MinFindPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, SteepestDescentSteps);
	DDX_Text(pDX, IDC_EDIT2, MinFindSteps);

	DDX_Radio(pDX, IDC_RADIO1, algorithm);
}


BEGIN_MESSAGE_MAP(MinFindPropertyPage, CMFCPropertyPage)
	ON_EN_CHANGE(IDC_EDIT1, &MinFindPropertyPage::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &MinFindPropertyPage::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_RADIO1, &MinFindPropertyPage::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &MinFindPropertyPage::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &MinFindPropertyPage::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &MinFindPropertyPage::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &MinFindPropertyPage::OnBnClickedRadio5)
END_MESSAGE_MAP()


// MinFindPropertyPage message handlers


BOOL MinFindPropertyPage::OnApply()
{
	UpdateData();
	ApplyValues();

	return CMFCPropertyPage::OnApply();
}


void MinFindPropertyPage::ApplyValues()
{
	theApp.options.SteepestDescentSteps = SteepestDescentSteps;
	theApp.options.MinFindSteps = MinFindSteps;
	theApp.options.algorithm = algorithm;

	theApp.options.Save();
}


void MinFindPropertyPage::OnEnChangeEdit1()
{
	SetModified();
}


void MinFindPropertyPage::OnEnChangeEdit2()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio1()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio2()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio3()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio4()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio5()
{
	SetModified();
}
