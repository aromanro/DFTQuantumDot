// DFTDoc.cpp : implementation of the CDFTDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DFT.h"
#endif

#include "DFTDoc.h"
#include "DFTView.h"

#include "VWNExcCor.h"

#include <numeric>


#include <propkey.h>

#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDFTDoc

IMPLEMENT_DYNCREATE(CDFTDoc, CDocument)

BEGIN_MESSAGE_MAP(CDFTDoc, CDocument)
	ON_COMMAND(ID_LEVEL_1, &CDFTDoc::OnLevel1)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_1, &CDFTDoc::OnUpdateLevel1)
	ON_COMMAND(ID_LEVEL_2, &CDFTDoc::OnLevel2)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_2, &CDFTDoc::OnUpdateLevel2)
	ON_COMMAND(ID_LEVEL_3, &CDFTDoc::OnLevel3)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_3, &CDFTDoc::OnUpdateLevel3)
	ON_COMMAND(ID_LEVEL_4, &CDFTDoc::OnLevel4)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_4, &CDFTDoc::OnUpdateLevel4)
	ON_COMMAND(ID_LEVEL_5, &CDFTDoc::OnLevel5)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_5, &CDFTDoc::OnUpdateLevel5)
	ON_COMMAND(ID_LEVEL_6, &CDFTDoc::OnLevel6)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_6, &CDFTDoc::OnUpdateLevel6)
	ON_COMMAND(ID_LEVEL_7, &CDFTDoc::OnLevel7)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_7, &CDFTDoc::OnUpdateLevel7)
	ON_COMMAND(ID_LEVEL_8, &CDFTDoc::OnLevel8)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_8, &CDFTDoc::OnUpdateLevel8)
	ON_COMMAND(ID_LEVEL_9, &CDFTDoc::OnLevel9)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_9, &CDFTDoc::OnUpdateLevel9)
	ON_COMMAND(ID_LEVEL_10, &CDFTDoc::OnLevel10)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_10, &CDFTDoc::OnUpdateLevel10)
END_MESSAGE_MAP()


// CDFTDoc construction/destruction

CDFTDoc::CDFTDoc()
	: displayLevel(0), 	
	computingFinished(false), computing(false)
{	
}

CDFTDoc::~CDFTDoc()
{
}

BOOL CDFTDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	SetTitle(L"No Data");

	return TRUE;
}




// CDFTDoc serialization

void CDFTDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CDFTDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CDFTDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CDFTDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CDFTDoc diagnostics

#ifdef _DEBUG
void CDFTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDFTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CDFTDoc commands


void CDFTDoc::OnLevel1()
{
	displayLevel = 0;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel1(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 0);

	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 1);
}


void CDFTDoc::OnLevel2()
{
	displayLevel = 1;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel2(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 1);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 2);
}

void CDFTDoc::OnLevel3()
{
	displayLevel = 2;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel3(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 2);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 3);
}

void CDFTDoc::OnLevel4()
{
	displayLevel = 3;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel4(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 3);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 4);
}

void CDFTDoc::OnLevel5()
{
	displayLevel = 4;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel5(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 4);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 5);
}

void CDFTDoc::OnLevel6()
{
	displayLevel = 5;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel6(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 5);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 6);
}

void CDFTDoc::OnLevel7()
{
	displayLevel = 6;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel7(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 6);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 7);
}

void CDFTDoc::OnLevel8()
{
	displayLevel = 7;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel8(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 7);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 8);
}

void CDFTDoc::OnLevel9()
{
	displayLevel = 8;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel9(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 8);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 9);
}

void CDFTDoc::OnLevel10()
{
	displayLevel = 9;
	UpdateAllViews(NULL);
}


void CDFTDoc::OnUpdateLevel10(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(displayLevel == 9);
	std::lock_guard<std::mutex> lock(resultsMutex);
	pCmdUI->Enable(results.options.Ns >= 10);
}



void CDFTDoc::StartComputing(const Options& options)
{
	if (computing.exchange(true)) return;

	SetTitle(L"Computing...");

	std::thread([this, options]() {

		// should I use this instead, or provide option to select? For molecules I get better results with VWN
		//DFT::DftSolver<DFT::VWNExchCor> dftSolver(options.dim1, options.dim2, options.dim3, options.samples1, options.samples2, options.samples3); 
		DFT::DftSolver<DFT::ChachiyoExchCor> dftSolver(options.dim1, options.dim2, options.dim3, options.samples1, options.samples2, options.samples3);		
		DFT::QuantumDot<decltype(dftSolver)> quantumDot(dftSolver, 1 == options.harmonic);
		dftSolver.fft.SetNumThreads(options.numThreadsFFT);

		QDResults computingResults;
		computingResults.options = options;
		computingResults.realSpaceCell = dftSolver.realSpaceCell;
		computingResults.reciprocalCell = dftSolver.reciprocalCell;



		Eigen::MatrixXcd W = Eigen::MatrixXcd::Random(dftSolver.realSpaceCell.Samples(), options.Ns);
		W = dftSolver.orthogonalize(W);

		// *************************** fdtest ************************************************

		/*
		double E0 = dftSolver.getE(W);

		TRACE(L"E0: %f\n", E0);

		Eigen::MatrixXcd g0 = dftSolver.getgrad(W);

		Eigen::MatrixXcd dW = Eigen::MatrixXcd::Random(dftSolver.realSpaceCell.Samples(), options.Ns);


		for (double delta = 10.; delta >= 1E-10; delta /= 10.)
		{
			double dE = 2. * delta *(g0.adjoint()*dW).trace().real();

			Eigen::MatrixXcd pos = W + delta*dW;

			TRACE(L"Actual/expected change: %20.16f, error estimate: %20.16f\n", (dftSolver.getE(pos) - E0)/dE, sqrt(dftSolver.realSpaceCell.Samples())*std::numeric_limits<double>::epsilon()/abs(dE));
		}
		*/

		// **********************************************************************************

		DFT::SolutionFinders::BaseClasses::SteepestDescent<decltype(dftSolver)> steepestDescent(dftSolver);
		W = steepestDescent.Descend(W, options.SteepestDescentSteps);

		W = dftSolver.orthogonalize(W);

		switch (options.algorithm)
		{
		case 0:
		{
			DFT::SolutionFinders::LineMinimization<decltype(dftSolver)> lmDescent(dftSolver);
			W = lmDescent.Descend(W, options.MinFindSteps);
		}
		break;
		case 1:
		{
			DFT::SolutionFinders::PreconditionedLineMinimization<decltype(dftSolver)> preLmDescent(dftSolver);
			W = preLmDescent.Descend(W, options.MinFindSteps);
		}
		break;
		case 2:
		{
			DFT::SolutionFinders::PolakRibiereConjugateGradient<decltype(dftSolver)> precgDescent(dftSolver);
			W = precgDescent.Descend(W, options.MinFindSteps);
		}
		break;
		case 3:
		{
			DFT::SolutionFinders::HestenesStiefelConjugateGradient<decltype(dftSolver)> precgDescent(dftSolver);
			W = precgDescent.Descend(W, options.MinFindSteps);
		}
		break;
		case 4:
		default:
		{
			DFT::SolutionFinders::FletcherReevesConjugateGradient<decltype(dftSolver)> precgDescent(dftSolver);
			W = precgDescent.Descend(W, options.MinFindSteps);
		}
		break;
		}

		computingResults.energies.resize(options.Ns);
		computingResults.wavefunctions.resize(options.Ns);

		auto ps = dftSolver.getPsi(W);
		for (int state = 0; state < options.Ns; ++state)
		{
			computingResults.energies[state] = ps.second(state).real();

			/*
			CString str;
			str.Format(L"State %d, energy: %f", state, computingResults.energies[state]);
			AfxMessageBox(str);
			*/

			Eigen::VectorXcd stateReciprocal = ps.first.col(state);
			Eigen::VectorXcd psi = dftSolver.cI(stateReciprocal);

			computingResults.wavefunctions[state] = psi;
		}

		SetResults(computingResults);

		computing = false;
		computingFinished = true;
	}).detach();
}


void CDFTDoc::SetResults(const QDResults& res)
{
	std::lock_guard<std::mutex> lock(resultsMutex);
	results = res;
}

