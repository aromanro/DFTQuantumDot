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
	numThreadsFFT = theApp.options.numThreadsFFT;
}

MinFindPropertyPage::~MinFindPropertyPage()
{
}

void MinFindPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, SteepestDescentSteps);
	DDX_Text(pDX, IDC_EDIT2, MinFindSteps);
	DDX_Text(pDX, IDC_EDIT3, numThreadsFFT);

	DDV_MinMaxUInt(pDX, SteepestDescentSteps, 20, 800);
	DDV_MinMaxUInt(pDX, MinFindSteps, 20, 800);
	DDV_MinMaxUInt(pDX, numThreadsFFT, 1, 128);

	DDX_Radio(pDX, IDC_RADIO1, algorithm);
}


BEGIN_MESSAGE_MAP(MinFindPropertyPage, CMFCPropertyPage)
	ON_EN_CHANGE(IDC_EDIT1, &MinFindPropertyPage::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT2, &MinFindPropertyPage::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT3, &MinFindPropertyPage::OnEnChangeEdit)
	ON_BN_CLICKED(IDC_RADIO1, &MinFindPropertyPage::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO2, &MinFindPropertyPage::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO3, &MinFindPropertyPage::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO4, &MinFindPropertyPage::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO5, &MinFindPropertyPage::OnBnClickedRadio)
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
	theApp.options.numThreadsFFT = numThreadsFFT;

	theApp.options.Save();
}


void MinFindPropertyPage::OnEnChangeEdit()
{
	SetModified();
}


void MinFindPropertyPage::OnBnClickedRadio()
{
	SetModified();
}

