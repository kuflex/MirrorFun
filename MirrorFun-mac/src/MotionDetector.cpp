#include "MotionDetector.h"

ofxCvGrayscaleImage 	grayImage;
ofxCvGrayscaleImage 	grayPrev;


CvMat *dx;
CvMat *dy;
CvMat *dx1;
CvMat *dy1;
CvMat *dUse;




//----------------------------------------------------------------
void MotionDetector::setup( int w, int h )
{
	_w = w;
	_h = h;
	_inited = false;
    
	grayImage.allocate( w, h );
	grayPrev.allocate( w, h );

	dx = cvCreateMat(h, w, CV_32FC1);
	dy = cvCreateMat(h, w, CV_32FC1); 
	dx1 = cvCreateMat(h, w, CV_32FC1);
	dy1 = cvCreateMat(h, w, CV_32FC1); 
	dUse = cvCreateMat(h, w, CV_32FC1);


	_outCell = 5;
	_outW = w / _outCell;
	_outH = h / _outCell;
	_outMotion = new ofPoint[ _outW * _outH ];
	

}

//----------------------------------------------------------------
void MotionDetector::update( ofxCvColorImage &colorImg )
{
	if ( !(_w == colorImg.width && _h == colorImg.height) ) {
		//Error
		return;
	}

	grayImage = colorImg;
	if ( !_inited ) {
		_inited = true;
		grayPrev = grayImage;
	}


	//Motion flow
	//Lucas and Kanade
	// compute dense Lucus-Kanade optical flow (previous, current)
	CvSize lucasSize = cvSize( 5, 5 );		//cvSize(5, 5), 			

	cvCalcOpticalFlowLK( grayPrev.getCvImage(), grayImage.getCvImage(), 
		lucasSize,
		dx, dy); 
	cvCalcOpticalFlowLK( grayImage.getCvImage(), grayPrev.getCvImage(), 
		lucasSize,
		dx1, dy1); 
		
	//смотреть - где отображаются далеко друг от друга - не использовать
	const int maxErrRad = 10*10;

	for (int y=0; y<_h; y++) {
		for (int x=0; x<_w; x++) {
			int fx = x + cvmGet( dx, y, x );
			int fy = y + cvmGet( dy, y, x );
			float use = 0;
			if ( fx >= 0 && fx < _w && fy >=0  && fy < _h ) {
				int ffx = fx + cvmGet( dx1, fy, fx );
				int ffy = fy + cvmGet( dy1, fy, fx );
				if ( (ffx - x)*(ffx - x) + (ffy - y)*(ffy - y) <= maxErrRad ) {
					use = 1;
				}
			}
			cvmSet( dUse, y, x, use );
		}
	}

	grayPrev = grayImage;


	//вычисление _outData
	const float minFill = 0.5;	//на сколько должна быть заполнена ячейка, чтоб ее учитывать

	int r = _outCell;
	const float minFillCount = r * r * minFill;

	for ( int y0 = 1; y0 < _outH - 1; y0++) {
		for (int x0 = 1; x0 < _outW - 1; x0++) {
			int x = x0 * _outCell;
			int y = y0 * _outCell;
			float vx = 0;
			float vy = 0; 
			int cnt = 0;
			for (int y1 = y - r; y1 < y + r; y1++ ) {
				for (int x1 = x - r; x1 < x + r; x1++ ) {
					if (cvmGet( dUse, y1, x1 ) > 0.5) {
						vx += cvmGet( dx, y1, x1 );
						vy += cvmGet( dy, y1, x1 );
						cnt ++;
					}
				}
			}
			if ( cnt > 0 
				&& cnt > minFillCount	//проверка, что точек достаточно много
				) {
				vx /= cnt;
				vy /= cnt;
			} else {
				vx = 0;
				vy = 0;
			}


			*outData( x0, y0 ) = ofPoint( vx, vy );
			//vx *= 2;
			//vy *= 2;
			//ofCircle( x0 + x * scale, y0 + y * scale, 1 );
			//ofLine( x0 + x * scale, y0 + y * scale, x0 + (x + vx) * scale, y0 + (y + vy) * scale );

		}
	}






	//cvAvg(dx)
}


//----------------------------------------------------------------
