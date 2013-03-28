
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

IMPLEMENT_DYNCREATE(COpenCVMFCView, CScrollView)

BEGIN_MESSAGE_MAP(COpenCVMFCView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &COpenCVMFCView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// COpenCVMFCView construction/destruction

CFile fCapture;
CFileException eCapture;
char pbuf[20];
int  captSetFlag=0;

COpenCVMFCView::COpenCVMFCView()
{
	// TODO: add construction code here
	saveImg    = NULL;
	workImg    = NULL;

	m_lpBmi    = 0;

	m_CaptFlag = 0;
	m_dibFlag  = 0;
	m_ImageType = 0;

	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

	if(fCapture.Open( "CaptSetup.txt", CFile::modeRead, &eCapture ) )
	{
		fCapture.Read( pbuf, 20 );          //  read resolution settings
		sscanf(pbuf,"%d  %d",&frameSetW,&frameSetH);
		fCapture.Close();
	}
}

COpenCVMFCView::~COpenCVMFCView()
{
	if (m_CaptFlag)
		AbortCapture(workImg);              //  close video

	if (saveImg)
		cvReleaseImage(&saveImg);           //  release image
	if (workImg)
		cvReleaseImage(&workImg);

	if (m_lpBmi)
		free(m_lpBmi);                      //  release bitmap info

	if (captSetFlag == 1) {
		if(fCapture.Open( "CaptSetup.txt", CFile::modeCreate | 
			CFile::modeWrite, &eCapture ) )
		{
			sprintf(pbuf,"%d  %d",frameSetW,frameSetH);
			fCapture.Write( pbuf, 20 );     //  write resolution settings
			fCapture.Close();
		}
		captSetFlag=0;
	}
}

BOOL COpenCVMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

// COpenCVMFCView drawing

void COpenCVMFCView::OnDraw(CDC* pDC)
{
	COpenCVMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here

	if (pDoc->pImg != NULL)	{	            //  got image to save to disk
		if (pDoc->m_Display == 0) {           //  not displayed yet
			imageClone(pDoc->pImg,&saveImg);         //  copy to backup image
			m_dibFlag = imageClone(saveImg,&workImg);  //  copy to work image

			m_ImageType = imageType(workImg);
			m_SaveFlag = m_ImageType;
			pDoc->m_Display = 1;
		}
	}

	if (m_dibFlag) {                        //  convert the DIB format
		if (m_lpBmi)
			free(m_lpBmi);
		m_lpBmi = CtreateMapInfo(workImg,m_dibFlag);
		m_dibFlag = 0;

		CSize  sizeTotal;
		sizeTotal = CSize(workImg->width,workImg->height);
		SetScrollSizes(MM_TEXT,sizeTotal);  //  set the scroll bar
	}

	char *pBits;
	if (m_CaptFlag==1)
		pBits=m_Frame->imageData;
	else if (workImg)
		pBits=workImg->imageData;

	if (workImg) {                          //  refresh UI
		StretchDIBits(pDC->m_hDC,
			0,0,workImg->width,workImg->height,
			0,0,workImg->width,workImg->height,
			pBits,m_lpBmi,DIB_RGB_COLORS,SRCCOPY);
	}
}

void COpenCVMFCView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
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
	CScrollView::AssertValid();
}

void COpenCVMFCView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

COpenCVMFCDoc* COpenCVMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COpenCVMFCDoc)));
	return (COpenCVMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// COpenCVMFCView message handlers
