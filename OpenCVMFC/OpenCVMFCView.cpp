
// OpenCVMFCView.cpp : implementation of the COpenCVMFCView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "OpenCVMFC.h"
#endif

#include "OpenCVMFCDoc.h"
#include "OpenCVMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COpenCVMFCView

IMPLEMENT_DYNCREATE(COpenCVMFCView, CView)

BEGIN_MESSAGE_MAP(COpenCVMFCView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &COpenCVMFCView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// COpenCVMFCView construction/destruction

COpenCVMFCView::COpenCVMFCView()
{
	// TODO: add construction code here

}

COpenCVMFCView::~COpenCVMFCView()
{
}

BOOL COpenCVMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// COpenCVMFCView drawing

void COpenCVMFCView::OnDraw(CDC* /*pDC*/)
{
	COpenCVMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// COpenCVMFCView printing


void COpenCVMFCView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL COpenCVMFCView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void COpenCVMFCView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void COpenCVMFCView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void COpenCVMFCView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void COpenCVMFCView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// COpenCVMFCView diagnostics

#ifdef _DEBUG
void COpenCVMFCView::AssertValid() const
{
	CView::AssertValid();
}

void COpenCVMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

COpenCVMFCDoc* COpenCVMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COpenCVMFCDoc)));
	return (COpenCVMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// COpenCVMFCView message handlers
