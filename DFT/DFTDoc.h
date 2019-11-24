
// DFTDoc.h : interface of the CDFTDoc class
//


#pragma once



#include <atomic>
#include <mutex>

#include "DftSolver.h"
#include "DftSolverCompressed.h"
#include "QuantumDot.h"
#include "SimpleDescent.h"


#include "Options.h"
#include "QDResults.h"




class CDFTView;

class CDFTDoc : public CDocument
{
protected: // create from serialization only
	CDFTDoc();
	DECLARE_DYNCREATE(CDFTDoc)

// Attributes
public:
	
	int displayLevel;

	std::atomic_bool computingFinished;
	std::atomic_bool computing;

	std::mutex resultsMutex;	
	QDResults results;


// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CDFTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnLevel1();
	afx_msg void OnUpdateLevel1(CCmdUI *pCmdUI);
	afx_msg void OnLevel2();
	afx_msg void OnUpdateLevel2(CCmdUI *pCmdUI);
	afx_msg void OnLevel3();
	afx_msg void OnUpdateLevel3(CCmdUI *pCmdUI);
	afx_msg void OnLevel4();
	afx_msg void OnUpdateLevel4(CCmdUI *pCmdUI);
	afx_msg void OnLevel5();
	afx_msg void OnUpdateLevel5(CCmdUI *pCmdUI);
	afx_msg void OnLevel6();
	afx_msg void OnUpdateLevel6(CCmdUI *pCmdUI);
	afx_msg void OnLevel7();
	afx_msg void OnUpdateLevel7(CCmdUI *pCmdUI);
	afx_msg void OnLevel8();
	afx_msg void OnUpdateLevel8(CCmdUI *pCmdUI);
	afx_msg void OnLevel9();
	afx_msg void OnUpdateLevel9(CCmdUI *pCmdUI);
	afx_msg void OnLevel10();
	afx_msg void OnUpdateLevel10(CCmdUI *pCmdUI);
	void StartComputing(const Options& options);
	void SetResults(const QDResults& res);
};
