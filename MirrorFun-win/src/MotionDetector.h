#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"


class MotionDetector
{
public:
	void setup( int w, int h );
	void update( ofxCvColorImage &colorImg );

	int outWidth()	{ return _outW; }
	int outHeight()	{ return _outH; }
	int outCell()	{ return _outCell; }
	ofPoint *outData( int x, int y ) {
		if ( x >= 0 && x < _outW && y >= 0 && y < _outH ) {
			return &_outMotion[ x + _outW * y ];
		}
		else {
			return 0;
		}
	}

private:
	int _w, _h;
	bool _inited;

	int _outCell; 
	int _outW, _outH; 
	ofPoint *_outMotion;

};
