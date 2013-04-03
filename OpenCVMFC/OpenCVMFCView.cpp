
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
	ON_UPDATE_COMMAND_UI(ID_WARP_AFFINE, &COpenCVMFCView::OnUpdateWarpAffine)
	ON_COMMAND(ID_WARP_AFFINE, &COpenCVMFCView::OnWarpAffine)
	ON_UPDATE_COMMAND_UI(ID_WARP_PERSPECT, &COpenCVMFCView::OnUpdateWarpPerspect)
	ON_COMMAND(ID_WARP_PERSPECT, &COpenCVMFCView::OnWarpPerspect)
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

	m_dibFlag = imageReplace(pImg8u,&workImg);

	imageClone(workImg,&saveImg);

	m_SaveFlag = m_ImageType = 1;

	cvReleaseImage( &pImg8u);

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

	pCmdUI->Enable((m_CaptFlag!=1)&&
		(m_ImageType)&&(m_ImageType!=-3));
}


void COpenCVMFCView::OnRotation30()
{
	// TODO: Add your command handler code here

	int angle = 30;                         //  Rotate 30 degree
	int opt = 0;                            //  1: with resize   0: just rotate
	double factor;                          //  resize factor
	IplImage *pImage;
	IplImage *pImgRotation = NULL;

	pImage = workImg;
	pImgRotation = cvCloneImage(workImg);

	angle = -angle;

	//  Create M Matrix
	float m[6];
	//      Matrix m looks like:
	//      [ m0  m1  m2 ] ----> [ a11  a12  b1 ]
	//      [ m3  m4  m5 ] ----> [ a21  a22  b2 ]

	CvMat M = cvMat(2,3,CV_32F,m);
	int w = workImg->width;
	int h = workImg->height;

	if (opt)
		factor = (cos(angle*CV_PI/180.)+1.0)*2;
	else 
		factor = 1;

	m[0] = (float)(factor*cos(-angle*CV_PI/180.));
	m[1] = (float)(factor*sin(-angle*CV_PI/180.));
	m[3] = -m[1];
	m[4] =  m[0];
	//  Make rotation center to image center
	m[2] = w*0.5f;
	m[5] = h*0.5f;

	//---------------------------------------------------------
	//  dst(x,y) = A * src(x,y) + b
	cvZero(pImgRotation);
	cvGetQuadrangleSubPix(pImage,pImgRotation,&M);
	//---------------------------------------------------------

	cvNamedWindow("Rotation Image");
	cvFlip(pImgRotation);
	cvShowImage("Rotation Image",pImgRotation);

	cvReleaseImage( &pImgRotation);

	cvWaitKey(0);

	cvDestroyWindow("Rotation Image");
}


void COpenCVMFCView::OnUpdateWarpAffine(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) &&
		(m_ImageType) && (m_ImageType!=-3));
}


void COpenCVMFCView::OnWarpAffine()
{
	// TODO: Add your command handler code here

	CvPoint2D32f srcTri[3], dstTri[3];
	CvMat* rot_mat  = cvCreateMat(2,3,CV_32FC1);
	CvMat* warp_mat = cvCreateMat(2,3,CV_32FC1);
	IplImage *src=0, *dst=0;

	src = cvCloneImage(workImg);
	cvFlip(src);
	dst = cvCloneImage(src);
	dst->origin = src->origin;
	cvZero(dst);

	//COMPUTE WARP MATRIX
	srcTri[0].x = 0;                          //src Top left
	srcTri[0].y = 0;
	srcTri[1].x = (float) src->width - 1;     //src Top right
	srcTri[1].y = 0;
	srcTri[2].x = 0;                          //src Bottom left
	srcTri[2].y = (float) src->height - 1;
	//- - - - - - - - - - - - - - -//
	dstTri[0].x = (float)(src->width*0.0);    //dst Top left
	dstTri[0].y = (float)(src->height*0.33);
	dstTri[1].x = (float)(src->width*0.85);   //dst Top right
	dstTri[1].y = (float)(src->height*0.25);
	dstTri[2].x = (float)(src->width*0.15);   //dst Bottom left
	dstTri[2].y = (float)(src->height*0.7);
	cvGetAffineTransform(srcTri,dstTri,warp_mat);
	cvWarpAffine(src,dst,warp_mat);
	cvCopy(dst,src);

	//COMPUTE ROTATION MATRIX
	CvPoint2D32f center = cvPoint2D32f(src->width/2,src->height/2);
	double angle = -50.0;
	double scale = 0.6;
	cv2DRotationMatrix(center,angle,scale,rot_mat);
	cvWarpAffine(src,dst,rot_mat);

	//DO THE TRANSFORM:
	cvNamedWindow( "Affine_Transform", 1 );
	cvShowImage( "Affine_Transform", dst );

	m_ImageType = -3;

	cvWaitKey();

	cvDestroyWindow( "Affine_Transform" );
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvReleaseMat(&rot_mat);
	cvReleaseMat(&warp_mat);

	m_ImageType=imageType(workImg);
}


void COpenCVMFCView::OnUpdateWarpPerspect(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) &&
		(m_ImageType) && (m_ImageType != -3));
}



void COpenCVMFCView::OnWarpPerspect()
{
	// TODO: Add your command handler code here

	CvPoint2D32f srcQuad[4], dstQuad[4];
	CvMat* warp_matrix = cvCreateMat(3,3,CV_32FC1);
	IplImage *src=0, *dst=0;

	src = cvCloneImage(workImg);
	cvFlip(src);
	dst = cvCloneImage(src);
	dst->origin = src->origin;
	cvZero(dst);

	srcQuad[0].x = 0;                         //src Top left
	srcQuad[0].y = 0;
	srcQuad[1].x = (float) src->width - 1;    //src Top right
	srcQuad[1].y = 0;
	srcQuad[2].x = 0;                         //src Bottom left
	srcQuad[2].y = (float) src->height - 1;
	srcQuad[3].x = (float) src->width - 1;    //src Bot right
	srcQuad[3].y = (float) src->height - 1;
	//- - - - - - - - - - - - - -//
	dstQuad[0].x = (float)(src->width*0.05);  //dst Top left
	dstQuad[0].y = (float)(src->height*0.33);
	dstQuad[1].x = (float)(src->width*0.9);   //dst Top right
	dstQuad[1].y = (float)(src->height*0.25);
	dstQuad[2].x = (float)(src->width*0.2);   //dst Bottom left
	dstQuad[2].y = (float)(src->height*0.7);      
	dstQuad[3].x = (float)(src->width*0.8);   //dst Bot right
	dstQuad[3].y = (float)(src->height*0.9);

	cvGetPerspectiveTransform(srcQuad,dstQuad,warp_matrix);
	cvWarpPerspective(src,dst,warp_matrix);

	cvNamedWindow( "Perspective_Warp", 1 );
	cvShowImage( "Perspective_Warp", dst );

	m_ImageType=-3;
	cvWaitKey();

	cvDestroyWindow( "Perspective_Warp" );
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvReleaseMat(&warp_matrix);

	m_ImageType=imageType(workImg);
}
