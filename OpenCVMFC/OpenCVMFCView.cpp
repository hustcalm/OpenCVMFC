
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
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &COpenCVMFCView::OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_REFRESH, &COpenCVMFCView::OnUpdateRefresh)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &COpenCVMFCView::OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_CONSERVATION_IMAGE, &COpenCVMFCView::OnUpdateConservationImage)
	ON_UPDATE_COMMAND_UI(ID_COLOR_IMAGE_REFRESH, &COpenCVMFCView::OnUpdateColorImageRefresh)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &COpenCVMFCView::OnUpdateFileSaveAs)
	ON_COMMAND(ID_REFRESH, &COpenCVMFCView::OnRefresh)
	ON_COMMAND(ID_CONSERVATION_IMAGE, &COpenCVMFCView::OnConservationImage)
	ON_COMMAND(ID_FILE_SAVE_AS, &COpenCVMFCView::OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_COLOR_TO_GRAY, &COpenCVMFCView::OnUpdateColorToGray)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_INVERT, &COpenCVMFCView::OnUpdateImageInvert)
	ON_COMMAND(ID_COLOR_TO_GRAY, &COpenCVMFCView::OnColorToGray)
	ON_COMMAND(ID_IMAGE_INVERT, &COpenCVMFCView::OnImageInvert)
	ON_COMMAND(ID_COLOR_IMAGE_REFRESH, &COpenCVMFCView::OnColorImageRefresh)
	ON_UPDATE_COMMAND_UI(ID_FLIP_V, &COpenCVMFCView::OnUpdateFlipV)
	ON_COMMAND(ID_FLIP_V, &COpenCVMFCView::OnFlipV)
	ON_UPDATE_COMMAND_UI(ID_FLIP_H, &COpenCVMFCView::OnUpdateFlipH)
	ON_COMMAND(ID_FLIP_H, &COpenCVMFCView::OnFlipH)
	ON_UPDATE_COMMAND_UI(ID_FLIP, &COpenCVMFCView::OnUpdateFlip)
	ON_COMMAND(ID_FLIP, &COpenCVMFCView::OnFlip)
	ON_UPDATE_COMMAND_UI(ID_ROTATION_30, &COpenCVMFCView::OnUpdateRotation30)
	ON_COMMAND(ID_ROTATION_30, &COpenCVMFCView::OnRotation30)
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

// Image Restore and Save

void COpenCVMFCView::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable(m_ImageType != -3);
}


void COpenCVMFCView::OnUpdateRefresh(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1)&&(m_ImageType != -3));
}


void COpenCVMFCView::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1)&&(m_ImageType != -3));
}


void COpenCVMFCView::OnUpdateConservationImage(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1)&&(m_ImageType != -3));
}


void COpenCVMFCView::OnUpdateColorImageRefresh(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1)&&(m_ImageType != -3));
}


void COpenCVMFCView::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1)&&(m_ImageType != -3));
}


void COpenCVMFCView::OnRefresh()
{
	// TODO: Add your command handler code here

	m_dibFlag = imageClone(saveImg,&workImg);
	m_ImageType = m_SaveFlag;

	Invalidate();
}

void COpenCVMFCView::OnColorImageRefresh()
{
	// TODO: Add your command handler code here

	COpenCVMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_Display = 0;

	Invalidate();
}

void COpenCVMFCView::OnConservationImage()
{
	// TODO: Add your command handler code here

	COpenCVMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_Display = 0;

	Invalidate();
}


void COpenCVMFCView::OnFileSaveAs()
{
	// TODO: Add your command handler code here

	CString csBMP = "BMP Files(*.BMP)|*.BMP|";
	CString csJPG = "JPEG Files(*.JPG)|*.JPG|";
	CString csTIF = "TIF Files(*.TIF)|*.TIF|";
	CString csPNG = "PNG Files(*.PNG)|*.PNG|";
	CString csDIB = "DIB Files(*.DIB)|*.DIB|";
	CString csPBM = "PBM Files(*.PBM)|*.PBM|";
	CString csPGM = "PGM Files(*.PGM)|*.PGM|";
	CString csPPM = "PPM Files(*.PPM)|*.PPM|";
	CString csSR  = "SR  Files(*.SR) |*.SR|";
	CString csRAS = "RAS Files(*.RAS)|*.RAS||";

	CString csFilter = csBMP+csJPG+csTIF+csPNG+csDIB
		+csPBM+csPGM+csPPM+csSR+csRAS;

	CString name[]={"","bmp","jpg","tif","png","dib",
		"pbm","pgm","ppm","sr", "ras",""};

	CString strFileName;
	CString strExtension;

	CFileDialog FileDlg(false,NULL,NULL,OFN_HIDEREADONLY,csFilter);		//  Save File Dialog

	if (FileDlg.DoModal() == IDOK ) {
		strFileName = FileDlg.m_ofn.lpstrFile;
		if (FileDlg.m_ofn.nFileExtension == 0) {  //  No Extension
			strExtension = name[FileDlg.m_ofn.nFilterIndex];
			strFileName = strFileName + '.' + strExtension;
			//  Add Extension
		}

		COpenCVMFCDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		pDoc->Save(strFileName,workImg);   //  Save The Image
	}
}

void COpenCVMFCView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	if (workImg) {                          //  Refresh Window
		CSize  sizeTotal;
		sizeTotal = CSize(workImg->width,workImg->height);
		SetScrollSizes(MM_TEXT, sizeTotal);   //  Set The Scroll Bar
	}
}

// Point Processing

void COpenCVMFCView::OnUpdateColorToGray(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag == 0)&&(m_ImageType > 1));
}


void COpenCVMFCView::OnUpdateImageInvert(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) &&
		(m_ImageType) && (m_ImageType != -3));
}


void COpenCVMFCView::OnColorToGray()
{
	// TODO: Add your command handler code here

	IplImage* pImage;
	IplImage* pImg8u = NULL;

	pImage = workImg;

	pImg8u = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);

	cvCvtColor(pImage, pImg8u, CV_BGR2GRAY);

	m_dibFlag=imageReplace(pImg8u,&workImg);

	imageClone(workImg,&saveImg);

	m_SaveFlag=m_ImageType=1;

	Invalidate();
}


void COpenCVMFCView::OnImageInvert()
{
	// TODO: Add your command handler code here

	cvNot(workImg,workImg);

	Invalidate();
}



void COpenCVMFCView::OnUpdateFlipV(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag!=1)&&
		(m_ImageType)&&(m_ImageType!=-3));
}

void COpenCVMFCView::OnFlipV()
{
	// TODO: Add your command handler code here

	cvFlip(workImg);                        //  Vertical Mirror cvFlip(in,0,0)

	Invalidate();
}


void COpenCVMFCView::OnUpdateFlipH(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag!=1)&&
		(m_ImageType)&&(m_ImageType!=-3));
}


void COpenCVMFCView::OnFlipH()
{
	// TODO: Add your command handler code here

	cvFlip(workImg,0,1);                    //  Horizonal Mirror

	Invalidate();
}


void COpenCVMFCView::OnUpdateFlip(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag!=1)&&
		(m_ImageType)&&(m_ImageType!=-3));
}


void COpenCVMFCView::OnFlip()
{
	// TODO: Add your command handler code here

	cvFlip(workImg,0,-1);                   //  180 degree rotation

	Invalidate();
}


void COpenCVMFCView::OnUpdateRotation30(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void COpenCVMFCView::OnRotation30()
{
	// TODO: Add your command handler code here
}
