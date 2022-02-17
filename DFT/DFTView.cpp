
// DFTView.cpp : implementation of the CDFTView class
//

#include "stdafx.h"


// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DFT.h"
#endif

//#include <vtkVolumeRayCastCompositeFunction.h>

#include "DFTDoc.h"
#include "DFTView.h"

#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);


#include <string>



ErrorObserver *ErrorObserver::New()
{
	return ::new ErrorObserver;
}

void ErrorObserver::Execute(vtkObject *vtkNotUsed(caller), unsigned long event, void *calldata)
{
	switch (event)
	{
	case vtkCommand::ErrorEvent:
		ErrorMessage = static_cast<char *>(calldata);
		Error = true;
		if (theView) theView->RecoverFromWarning();
		break;
	case vtkCommand::WarningEvent:
		WarningMessage = static_cast<char *>(calldata);
		Warning = true;
		if (theView) theView->RecoverFromWarning();
		break;
	}
}


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




// CDFTView

IMPLEMENT_DYNCREATE(CDFTView, CView)

BEGIN_MESSAGE_MAP(CDFTView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDFTView::OnFilePrintPreview)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CDFTView construction/destruction

CDFTView::CDFTView()
	: ren(nullptr), iren(nullptr), renWin(nullptr), volumeMapper(nullptr), volume(nullptr), timer(NULL)
{
	errorObserver = ErrorObserver::New();
	errorObserver->theView = this;

	// avoid displaying VTK warning window and log the messages instead
	vtkSmartPointer<vtkFileOutputWindow> outputWin = vtkSmartPointer<vtkFileOutputWindow>::New();
	outputWin->SetFileName("vtkErrors.log");
	vtkOutputWindow::SetInstance(outputWin);

	outputWin->AddObserver(vtkCommand::ErrorEvent, errorObserver);
	outputWin->AddObserver(vtkCommand::WarningEvent, errorObserver);


	volumeMapper = vtkGPUVolumeRayCastMapper::New();
	volume = vtkVolume::New();
}

CDFTView::~CDFTView()
{
	// Delete the renderer, window and interactor objects.

	if (ren)
	{
		ren->Delete();
		iren->Delete();
		renWin->Delete();
	}


	// Delete the objects used to form the visualization.

	volumeMapper->Delete();
	volume->Delete();

	errorObserver->Delete();

	for (unsigned int i = 0; i < dataImage.size(); ++i)
		dataImage[i]->Delete();
}

// CDFTView drawing

void CDFTView::OnDraw(CDC* pDC)
{
	const CDFTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	if (!iren || !renWin)return;

	if (!iren->GetInitialized())
	{
		iren->SetRenderWindow(renWin);

		CRect rect;

		GetClientRect(&rect);
		iren->Initialize();
		renWin->SetSize(rect.right - rect.left, rect.bottom - rect.top);
		ren->ResetCamera();
	}

	// Invoke the pipeline
	Pipeline();

	if (pDC->IsPrinting())
	{
		BeginWaitCursor();

		// Obtain the size of the printer page in pixels.

		const int cxPage = pDC->GetDeviceCaps(HORZRES);
		const int cyPage = pDC->GetDeviceCaps(VERTRES);

		// Get the size of the window in pixels.

		const int *size = renWin->GetSize();
		int cxWindow = size[0];
		int cyWindow = size[1];
		const float fx = static_cast<float>(cxPage) / static_cast<float>(cxWindow);
		const float fy = static_cast<float>(cyPage) / static_cast<float>(cyWindow);
		const float scale = min(fx, fy);
		const int x = static_cast<int>(scale * static_cast<float>(cxWindow));
		const int y = static_cast<int>(scale * static_cast<float>(cyWindow));

		// this is from VTK-8.0.1\GUISupport\MFC\vtkMFCWindow.cpp
		// with some corrections, they don't do DeleteObject and DeleteDC and here for some reason delete[] pixels crashes

		renWin->SetUseOffScreenBuffers(true);
		renWin->Render();

		const unsigned char *pixels = renWin->GetPixelData(0, 0, cxWindow - 1, cyWindow - 1, 0);

		int dataWidth = ((cxWindow * 3 + 3) / 4) * 4;

		BITMAPINFO MemoryDataHeader;
		MemoryDataHeader.bmiHeader.biSize = 40;
		MemoryDataHeader.bmiHeader.biWidth = cxWindow;
		MemoryDataHeader.bmiHeader.biHeight = cyWindow;
		MemoryDataHeader.bmiHeader.biPlanes = 1;
		MemoryDataHeader.bmiHeader.biBitCount = 24;
		MemoryDataHeader.bmiHeader.biCompression = BI_RGB;
		MemoryDataHeader.bmiHeader.biClrUsed = 0;
		MemoryDataHeader.bmiHeader.biClrImportant = 0;
		MemoryDataHeader.bmiHeader.biSizeImage = dataWidth*cyWindow;
		MemoryDataHeader.bmiHeader.biXPelsPerMeter = 10000;
		MemoryDataHeader.bmiHeader.biYPelsPerMeter = 10000;

		unsigned char *MemoryData = NULL;
		HDC MemoryHdc = static_cast<HDC>(CreateCompatibleDC(pDC->GetSafeHdc()));
		HBITMAP dib = CreateDIBSection(MemoryHdc, &MemoryDataHeader, DIB_RGB_COLORS, (void **)(&MemoryData), NULL, 0);
		if (dib)
		{
			// copy the pixels over
			if (MemoryData)
			{
				for (int i = 0; i < cyWindow; ++i)
					for (int j = 0; j < cxWindow; ++j)
					{
						MemoryData[i * dataWidth + j * 3] = pixels[i * cxWindow * 3 + j * 3 + 2];
						MemoryData[i * dataWidth + j * 3 + 1] = pixels[i * cxWindow * 3 + j * 3 + 1];
						MemoryData[i * dataWidth + j * 3 + 2] = pixels[i * cxWindow * 3 + j * 3];
					}
			}

			SelectObject(MemoryHdc, dib);
			StretchBlt(pDC->GetSafeHdc(), 0, 0, x, y, MemoryHdc, 0, 0, cxWindow, cyWindow, SRCCOPY);

			renWin->SetUseOffScreenBuffers(false);

			DeleteObject(dib);
		}
		DeleteDC(MemoryHdc);

		// in debug this crashes
#ifndef _DEBUG
		delete[] pixels;
#endif

		EndWaitCursor();
	}
	else
	{
		renWin->Render();
	}
}


// CDFTView printing


void CDFTView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CDFTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CDFTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CDFTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

// CDFTView diagnostics

#ifdef _DEBUG
void CDFTView::AssertValid() const
{
	CView::AssertValid();
}

void CDFTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDFTDoc* CDFTView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDFTDoc)));
	return dynamic_cast<CDFTDoc*>(m_pDocument);
}
#endif //_DEBUG


// CDFTView message handlers


void CDFTView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CRect rect;

	GetClientRect(&rect);

	if (renWin)
	{
		renWin->SetSize(rect.right - rect.left, rect.bottom - rect.top);
		if (iren) iren->UpdateSize(rect.Width(), rect.Height());
	}
}


BOOL CDFTView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}


void CDFTView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// Create the renderer, window and interactor objects.

	ren = vtkRenderer::New();
	renWin = vtkWin32OpenGLRenderWindow::New();
	iren = vtkWin32RenderWindowInteractor::New();

	renWin->AddRenderer(ren);


	// setup the parent window

	renWin->SetParentId(GetSafeHwnd());

	iren->SetRenderWindow(renWin);

	// now the other ones

	ren->SetBackground(0.1, 0.3, 0.4);

	// camera

	ren->GetActiveCamera()->SetFocalPoint(results.realSpaceCell.GetSize().X / 2, results.realSpaceCell.GetSize().Y / 2, results.realSpaceCell.GetSize().Z / 2);
	ren->GetActiveCamera()->SetPosition(results.realSpaceCell.GetSize().X, results.realSpaceCell.GetSize().Y * 2, results.realSpaceCell.GetSize().Z * 3);
	ren->GetActiveCamera()->ComputeViewPlaneNormal();

	// text

	textActor = vtkSmartPointer<vtkTextActor>::New();
	textActor->SetInput("Set options then start computing with File|Calculate");
	textActor->SetPosition(10, 10);
	textActor->GetTextProperty()->SetFontSize(36);
	textActor->GetTextProperty()->SetColor(0., 1.0, 0.);
	ren->AddActor2D(textActor);

	// THE VOLUME MAPPER FOR THE ORBITAL

	volumeMapper->SetSampleDistance(results.realSpaceCell.GetSize().X / results.realSpaceCell.GetSamples().X / 4);
	//volumeMapper->SetReportProgress(0);


	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetAutoAdjustSampleDistances(0);

	vtkVolumeProperty* volumeProperty = volume->GetProperty();

	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->SetColorSpaceToRGB();
	colorTransferFunction->AddRGBPoint(0, 1., 0, 0);
	colorTransferFunction->AddRGBPoint(results.maxs[1], 1., 0, 0);
	volumeProperty->SetColor(colorTransferFunction);


	volumeProperty->ShadeOn();
	//volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationTypeToLinear();

	volume->SetMapper(volumeMapper);

	ren->AddViewProp(volume);


	// initialize the interactor

	CRect rect;

	GetClientRect(&rect);
	iren->Initialize();
	renWin->SetSize(rect.right - rect.left, rect.bottom - rect.top);

	timer = SetTimer(1, 300, NULL);
}

bool IsHandledMessage(UINT message)
{
	return message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MBUTTONDOWN || message == WM_MBUTTONUP ||
		message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_MOUSEMOVE || message == WM_CHAR || message == WM_TIMER;
}

LRESULT CDFTView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (IsHandledMessage(message))
	{
		if (iren && iren->GetInitialized() && GetSafeHwnd())
		{
			LRESULT res  = vtkHandleMessage(GetSafeHwnd(), message, wParam, lParam);
			//LRESULT res = vtkHandleMessage2(GetSafeHwnd(), message, wParam, lParam, iren);
			if (message != WM_TIMER || wParam != timer) return res;
		}
	}

	return CView::WindowProc(message, wParam, lParam);
}

void CDFTView::RecoverFromWarning()
{
	CDFTDoc* pDoc = GetDocument();
	if (!pDoc || saveLastViewOrbital == -1) return;

	const int saveLevel = pDoc->displayLevel;
	pDoc->displayLevel = saveLastViewOrbital;
	saveLastViewOrbital = saveLevel;

	pDoc->UpdateAllViews(NULL);
}

void CDFTView::Pipeline()
{
	const CDFTDoc* pDoc = GetDocument();
	if (!pDoc) return;

	int i = pDoc->displayLevel;

	if (i == lastViewOrbital || static_cast<int>(dataImage.size()) <= i) return;
	else 
	{
		saveLastViewOrbital = lastViewOrbital;
		lastViewOrbital = i;
	}
	

	ren->RemoveAllViewProps();

	vtkVolumeProperty* volumeProperty = volume->GetProperty();

	vtkPiecewiseFunction* opacityTransferFunction = volumeProperty->GetScalarOpacity();
	opacityTransferFunction->RemoveAllPoints();

	opacityTransferFunction->AddPoint(0, 0);
	opacityTransferFunction->AddPoint(results.maxs[i], 1);


	volumeMapper->SetInputData(dataImage[i]);	

	ren->AddViewProp(volume);

	vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
	cube->SetBounds(0, results.realSpaceCell.GetSize().X, 0, results.realSpaceCell.GetSize().Y, 0, results.realSpaceCell.GetSize().Z);


	vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	//vtkSmartPointer<vtkOutlineFilter> outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	//outlineFilter->SetInputConnection(cube->GetOutputPort());
	//outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
	outlineMapper->SetInputConnection(cube->GetOutputPort());

	outlineActor->SetMapper(outlineMapper);
	outlineActor->GetProperty()->SetRepresentationToWireframe();
	outlineActor->GetProperty()->SetColor(1, 1, 1);

	ren->AddActor(outlineActor);

	textActor = vtkSmartPointer<vtkTextActor>::New();
	CString str;
	str.Format(L"E: %.2f Hartree", results.energies[i]);
	USES_CONVERSION;
	textActor->SetInput(W2A(str));
	textActor->SetPosition(10, 10);
	textActor->GetTextProperty()->SetFontSize(36);
	textActor->GetTextProperty()->SetColor(0., 1.0, 0.);
	ren->AddActor2D(textActor);
}


void CDFTView::OnDestroy()
{
	CView::OnDestroy();

	if (timer) KillTimer(timer);
}


void CDFTView::OnTimer(UINT_PTR nIDEvent)
{
	CView::OnTimer(nIDEvent);

	if (timer == nIDEvent)
	{
		CDFTDoc* pDoc = GetDocument();
		if (!pDoc || !pDoc->computingFinished) return;
		pDoc->computingFinished = false; // set it to false to avoid getting the results again

		GrabResultsFromDoc();

		pDoc->SetTitle(L"Orbitals");

		pDoc->UpdateAllViews(NULL);
	}
}




void CDFTView::GrabResultsFromDoc()
{
	CDFTDoc* pDoc = GetDocument();
	if (!pDoc) return;

	{
		std::lock_guard<std::mutex> lock(pDoc->resultsMutex);
		results = pDoc->results;
		pDoc->results.FreeSomeMemory(); // will never look at it until overwritten by a new computation
	}

	// force doc displayLevel and so on, it's a new orbitals display
	pDoc->displayLevel = 0;
	lastViewOrbital = -1;
	saveLastViewOrbital = -1;

	// now build the data images

	for (unsigned int i = 0; i < dataImage.size(); ++i)
		dataImage[i]->Delete();


	dataImage.resize(results.wavefunctions.size());
	results.maxs.resize(results.wavefunctions.size());
	for (unsigned int state = 0; state < results.wavefunctions.size(); ++state)
	{
		dataImage[state] = vtkImageData::New();

		dataImage[state]->SetSpacing(results.realSpaceCell.GetSize().X/results.realSpaceCell.GetSamples().X, results.realSpaceCell.GetSize().Y/results.realSpaceCell.GetSamples().Y, results.realSpaceCell.GetSize().Z/results.realSpaceCell.GetSamples().Z);
		dataImage[state]->SetDimensions(results.realSpaceCell.GetSamples().X, results.realSpaceCell.GetSamples().Y, results.realSpaceCell.GetSamples().Z); //number of points in each direction

		dataImage[state]->AllocateScalars(VTK_FLOAT, 1);

		unsigned int index = 0;
		results.maxs[state] = 0;
		for (unsigned int i = 0; i < results.realSpaceCell.GetSamples().X; ++i)
			for (unsigned int j = 0; j < results.realSpaceCell.GetSamples().Y; ++j)
				for (unsigned int k = 0; k < results.realSpaceCell.GetSamples().Z; ++k)
				{
					const double val = std::norm(results.wavefunctions[state](index));
					dataImage[state]->SetScalarComponentFromDouble(i, j, k, 0, val);

					if (val > results.maxs[state]) results.maxs[state] = val;
					if (val > results.theMax) results.theMax = val;
					++index;
				}
	}
}
