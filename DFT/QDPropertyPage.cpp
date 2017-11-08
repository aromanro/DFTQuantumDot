// QDPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "QDPropertyPage.h"
#include "afxdialogex.h"

#include "resource.h"

#include "DFT.h"


// QDPropertyPage dialog

IMPLEMENT_DYNAMIC(QDPropertyPage, CMFCPropertyPage)

QDPropertyPage::QDPropertyPage()
	: CMFCPropertyPage(IDD_PROPPAGE_QD)
{
	dim1 = theApp.options.dim1;
	dim2 = theApp.options.dim2;
	dim3 = theApp.options.dim3;

	samples1 = theApp.options.samples1;
	samples2 = theApp.options.samples2;
	samples3 = theApp.options.samples3;
	Ns = theApp.options.Ns;

	harmonic = theApp.options.harmonic;
}

QDPropertyPage::~QDPropertyPage()
{
}

void QDPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT1, dim1Edit);
	DDX_Control(pDX, IDC_EDIT2, dim2Edit);
	DDX_Control(pDX, IDC_EDIT3, dim2Edit);

	DDX_Text(pDX, IDC_EDIT1, dim1);
	DDX_Text(pDX, IDC_EDIT2, dim2);
	DDX_Text(pDX, IDC_EDIT3, dim3);

	DDX_Text(pDX, IDC_EDIT4, samples1);
	DDX_Text(pDX, IDC_EDIT5, samples2);
	DDX_Text(pDX, IDC_EDIT6, samples3);

	DDX_Radio(pDX, IDC_RADIO1, harmonic);

	DDX_Text(pDX, IDC_EDIT7, Ns);

	DDV_MinMaxDouble(pDX, dim1, 0.1, 100.);
	DDV_MinMaxDouble(pDX, dim2, 0.1, 100.);
	DDV_MinMaxDouble(pDX, dim3, 0.1, 100.);
	DDV_MinMaxInt(pDX, samples1, 16, 128);
	DDV_MinMaxInt(pDX, samples2, 16, 128);
	DDV_MinMaxInt(pDX, samples3, 16, 128);

	DDV_MinMaxInt(pDX, Ns, 1, 10);
}


BEGIN_MESSAGE_MAP(QDPropertyPage, CMFCPropertyPage)
	ON_EN_CHANGE(IDC_EDIT1, &QDPropertyPage::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &QDPropertyPage::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &QDPropertyPage::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &QDPropertyPage::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &QDPropertyPage::OnEnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT6, &QDPropertyPage::OnEnChangeEdit6)
	ON_BN_CLICKED(IDC_RADIO1, &QDPropertyPage::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &QDPropertyPage::OnBnClickedRadio2)
	ON_EN_CHANGE(IDC_EDIT7, &QDPropertyPage::OnEnChangeEdit7)
END_MESSAGE_MAP()


// QDPropertyPage message handlers


BOOL QDPropertyPage::OnApply()
{
	UpdateData();
	ApplyValues();

	return CMFCPropertyPage::OnApply();
}


BOOL QDPropertyPage::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	dim1Edit.allowNegative = false;
	dim2Edit.allowNegative = false;
	dim3Edit.allowNegative = false;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void QDPropertyPage::ApplyValues()
{
	theApp.options.dim1 = dim1;
	theApp.options.dim2 = dim2;
	theApp.options.dim3 = dim3;

	theApp.options.samples1 = samples1;
	theApp.options.samples2 = samples2;
	theApp.options.samples3 = samples3;
	theApp.options.Ns = Ns;

	theApp.options.harmonic = harmonic;

	theApp.options.Save();
}


void QDPropertyPage::OnEnChangeEdit1()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit2()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit3()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit4()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit5()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit6()
{
	SetModified();
}


void QDPropertyPage::OnBnClickedRadio1()
{
	SetModified();
}


void QDPropertyPage::OnBnClickedRadio2()
{
	SetModified();
}


void QDPropertyPage::OnEnChangeEdit7()
{
	SetModified();
}
