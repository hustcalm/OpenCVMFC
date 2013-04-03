
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
	ON_UPDATE_COMMAND_UI(ID_IMAGE_ADJUST, &COpenCVMFCView::OnUpdateImageAdjust)
	ON_COMMAND(ID_IMAGE_ADJUST, &COpenCVMFCView::OnImageAdjust)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_HISTOGRAM, &COpenCVMFCView::OnUpdateImageHistogram)
	ON_COMMAND(ID_IMAGE_HISTOGRAM, &COpenCVMFCView::OnImageHistogram)
	ON_UPDATE_COMMAND_UI(ID_HIST_EQUALIZE, &COpenCVMFCView::OnUpdateHistEqualize)
	ON_COMMAND(ID_HIST_EQUALIZE, &COpenCVMFCView::OnHistEqualize)
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

	//cvReleaseImage( &pImage);
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


void COpenCVMFCView::OnUpdateImageAdjust(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) && (m_ImageType == 1));
}

//  Image Adjust

/*
  src and dst are grayscale, 8-bit images;
  Default input value: 
           [low, high] = [0,1];  X-Direction
           [bottom, top] = [0,1]; Y-Direction
           gamma ;
  if adjust successfully, return 0, otherwise, return non-zero.
*/
int ImageAdjust(IplImage* src, IplImage* dst, 
    	double low, double high,   // X£ºlow and high are the intensities of src
    	double bottom, double top, // Y£ºmapped to bottom and top of dst
    	double gamma )
{
	if (low<0 && low>1 && high <0 && high>1&&
		bottom<0 && bottom>1 && top<0 && top>1 && low>high)
        return -1;

    double low2 = low*255;
    double high2 = high*255;
    double bottom2 = bottom*255;
    double top2 = top*255;
    double err_in = high2 - low2;
    double err_out = top2 - bottom2;

    int x,y;
    double val;

    // intensity transform
    for( y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            val = ((uchar*)(src->imageData + src->widthStep*y))[x]; 
            val = pow((val - low2)/err_in, gamma) * err_out + bottom2;
            if(val > 255) val = 255; if(val < 0) val = 0; // Make sure src is in the range [low,high]
            ((uchar*)(dst->imageData + dst->widthStep*y))[x] = (uchar) val;
        }
    }
    return 0;
}


void COpenCVMFCView::OnImageAdjust()
{
	// TODO: Add your command handler code here

	IplImage *src = 0, *dst = 0;

	src = workImg;

	cvNamedWindow( "src", 1 );
	cvNamedWindow( "result", 1 );

	// Image adjust
	dst = cvCloneImage(src);
	// Input parameter [0,0.5] and [0.5,1], gamma=1
	if( ImageAdjust( src, dst, 0, 0.5, 0.5, 1, 1) != 0) 
		return;

	cvShowImage( "src", src );
	cvFlip(dst);
	cvShowImage( "result", dst );
	cvWaitKey(0);

	cvDestroyWindow("src");
	cvDestroyWindow("result");

	cvFlip(dst);
	m_dibFlag = imageReplace(dst, &workImg);

	//cvReleaseImage( &src );
	cvReleaseImage(&dst);

	Invalidate();
}


void COpenCVMFCView::OnUpdateImageHistogram(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) && (m_ImageType == 1));
}


void COpenCVMFCView::OnImageHistogram()
{
	// TODO: Add your command handler code here

	IplImage *src;
	IplImage *histimg = 0;
	CvHistogram *hist = 0;

	int hdims = 256;     // Divide Number of HIST, the more the accurator
	float hranges_arr[] = {0,255};
	float* hranges = hranges_arr;
	int bin_w;  
	float max_val;
	int i;

	src = workImg;

	cvNamedWindow( "Histogram", 0 );

	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  // Create Hist
	histimg = cvCreateImage( cvSize(320,200), 8, 3 );
	cvZero( histimg );
	cvCalcHist( &src, hist, 0, 0 ); // Caculate Hist
	cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );  // just wanna the Max
	cvConvertScale( hist->bins, hist->bins, 
		max_val ? 255. / max_val : 0., 0 ); // resize bin to [0,255] 
	cvZero( histimg );
	bin_w = histimg->width / hdims;

	// Draw It
	for( i = 0; i < hdims; i++ )
	{
		double val = ( cvGetReal1D(hist->bins,i)*histimg->height/255 );
		CvScalar color = CV_RGB(255,255,0); //(hsv2rgb(i*180.f/hdims);
		cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
			cvPoint((i+1)*bin_w,(int)(histimg->height - val)),
			color, 1, 8, 0 );
	}

	cvShowImage( "Histogram", histimg );

	cvReleaseImage( &histimg );
	cvReleaseHist ( &hist );
	cvWaitKey(0);

	cvDestroyWindow("Histogram");
}


void COpenCVMFCView::OnUpdateHistEqualize(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable((m_CaptFlag != 1) && (m_ImageType == 1));
}

#define HDIM    256    // bin of HIST, default = 256

void COpenCVMFCView::OnHistEqualize()
{
	// TODO: Add your command handler code here

	IplImage *src = 0, *dst = 0;
	CvHistogram *hist = 0;

	int n = HDIM;     
	double nn[HDIM];
	uchar T[HDIM];
	CvMat *T_mat;

	int x;
	int sum = 0; // sum of pixels of the source image
	double val = 0;

	src = workImg;

	cvNamedWindow( "source", 1 );
	cvNamedWindow( "result", 1 );

	// Caculate Hist
	hist = cvCreateHist( 1, &n, CV_HIST_ARRAY, 0, 1 );  
	cvCalcHist( &src, hist, 0, 0 ); 

	// Create Accumulative Distribute Function of histgram
	val = 0;
	for ( x = 0; x < n; x++)
	{
		val = val + cvGetReal1D (hist->bins, x);
		nn[x] = val;
	}

	// Normalization
	sum = src->height * src->width;
	for( x = 0; x < n; x++ )
	{
		T[x] = (uchar) (255 * nn[x] / sum); // range is [0,255]
	}

	// Using look-up table to perform intensity transform for source image 
	dst = cvCloneImage( src );
	T_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
	cvSetData( T_mat, T, 0 );    
	// invoke to complete look-up-table
	cvLUT( src, dst, T_mat ); 

	cvShowImage( "source", src );
	cvFlip(dst);
	cvShowImage( "result", dst );

	cvReleaseHist ( &hist );

	cvWaitKey(0);

	cvDestroyWindow("source");
	cvDestroyWindow("result");

	cvFlip(dst);
	m_dibFlag = imageReplace(dst,&workImg);

	cvReleaseMat(&T_mat);

	//cvReleaseImage( &src );
	cvReleaseImage( &dst );

	Invalidate();
}
