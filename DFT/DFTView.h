
// DFTView.h : interface of the CDFTView class
//

#pragma once


#ifdef _DEBUG
#define new ::new
#endif

#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkCubeSource.h>

#include <vtkFileOutputWindow.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <vtkFloatArray.h>
#include <vtkImageData.h>

#include <vtkDataSetMapper.h>

#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkInformation.h>

#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkWin32OutputWindow.h>
#include <vtkCommand.h>


#ifdef _DEBUG
#undef new
#endif


#include "QDResults.h"




class CDFTView;


// class stolen from VTK examples and modified

class ErrorObserver : public vtkCommand
{
public:
	ErrorObserver() : theView(nullptr), Error(false), Warning(false), ErrorMessage(""), WarningMessage("") {}
  
	static ErrorObserver *New();

	bool GetError() const
	{
		return Error;
	}
	
	bool GetWarning() const
	{
		return Warning;
	}
	
	void Clear()
	{
		Error = false;
		Warning = false;
		ErrorMessage = "";
		WarningMessage = "";
	}

	virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long event, void *calldata);

	std::string GetErrorMessage() const
	{
		return ErrorMessage;
	}
	
	std::string GetWarningMessage() const
	{
		return WarningMessage;
	}

	CDFTView* theView;

private:
	bool        Error;
	bool        Warning;
	std::string ErrorMessage;
	std::string WarningMessage;
};


class CDFTView : public CView
{
protected: // create from serialization only
	CDFTView();
	DECLARE_DYNCREATE(CDFTView)

// Attributes
public:
	CDFTDoc* GetDocument() const;

// Operations
public:
	int lastViewOrbital = -1;
	int saveLastViewOrbital = -1; // for recovery

	ErrorObserver* errorObserver = nullptr;

	std::vector<vtkImageData*> dataImage;

	QDResults results;
	
	void RecoverFromWarning();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CDFTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	vtkWin32OpenGLRenderWindow *renWin;
	vtkRenderer *ren;
	vtkWin32RenderWindowInteractor *iren;

	vtkGPUVolumeRayCastMapper* volumeMapper;
	vtkVolume* volume;

	vtkSmartPointer<vtkTextActor> textActor;

	UINT_PTR timer;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void Pipeline();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void GrabResultsFromDoc();
};

#ifndef _DEBUG  // debug version in DFTView.cpp
inline CDFTDoc* CDFTView::GetDocument() const
   { return reinterpret_cast<CDFTDoc*>(m_pDocument); }
#endif

