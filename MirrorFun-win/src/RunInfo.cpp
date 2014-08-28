#include "RunInfo.h"
#include "ofxIniSettings.h"

RunInfo runInfo;

//--------------------------------------------------------------
string screenFormatToStr( pbScreenFormat format ) 
{
	string res = "";
	if ( format == pbScreenFormat_4x3 ) res = "4x3";
	if ( format == pbScreenFormat_16x9 ) res = "16x9";
	if ( format == pbScreenFormat_16x10 ) res = "16x10";
	return res;
}

pbScreenFormat screenFormatFromStr( const string &str ) 
{
	pbScreenFormat format = pbScreenFormat_4x3;
	if ( str == "4x3" ) format = pbScreenFormat_4x3;
	if ( str == "16x9" ) format = pbScreenFormat_16x9;
	if ( str == "16x10" ) format = pbScreenFormat_16x10;
	return format;
}


//--------------------------------------------------------------
RunInfo::RunInfo(void)
{	
	_w = 600;
	_h = 800;
	_screenFormat = pbScreenFormat_4x3;
	_rotate = 0;	
	_mirror = 0;
	_resizeMode = ResizeModeCrop;
	_fullScreen=0;	
	_runDurationSec = 0.0;
	_soundVolume = 1.0;
	_cameraDevice = 0;
	_cameraDevice2 = 1;
	_cameraFrameRate = 30;
	_cameraMirror = 0;
	_cameraGrabW = 320;
	_cameraGrabH = 240;
	_cameraProcessW = 320;
	_cameraProcessH = 240;
	_cameraOptFlowW = 320;
	_cameraOptFlowH = 240;


	_durationControl = true;
	_fadeDuration = 0.0;
	_introDuration = 0.0;

	_printFps		= false;	
	_updateStart	= 0.0;
	_updateEnd		= 0.0;
}

//--------------------------------------------------------------
void RunInfo::setup() 
{
	//cout << "--- You can use command line parameters : --------" << endl;
	//cout << "-SettingsFile=[filename] -Screen.scale=[0.1...1.0] -Run.durationSec=[1..10000]" << endl;

	float scale = 1.0;


	//Файл настроек
	ofxIniSettings ini;
	ini.load( ofToDataPath("settings.ini") );

	_w				= ini.get("Screen.w", _w);
	_h				= ini.get("Screen.h", _h);
	_screenFormat	= screenFormatFromStr( ini.get("Screen.screenFormat", string("") ) );
	_fullScreen		= ini.get("Screen.fullScreen", _fullScreen);
	_rotate			= ini.get("Screen.rotate", _rotate);
	_mirror			= ini.get("Screen.mirror", _mirror);
	_fadeDuration	= ini.get("Screen.fadeDuration", _fadeDuration );
	_introDuration	= ini.get("Screen.introDuration", _introDuration );
	_introImageFile		= ini.get("Screen.introImage", string("") );


	string resizeMode = ini.get("Screen.resizeMode", string("") );
	if ( resizeMode == "crop" ) _resizeMode = ResizeModeCrop;
	if ( resizeMode == "fit" ) _resizeMode = ResizeModeFit;

	//Звук
	_soundVolume = ini.get( "Sound.volume", _soundVolume);

	//Камера
	_cameraDevice	= ini.get( "Camera.device", _cameraDevice );
	_cameraDevice2	= ini.get( "Camera.device2", _cameraDevice );
	_cameraFrameRate	= ini.get( "Camera.frameRate", _cameraFrameRate );
	_cameraMirror	= ini.get( "Camera.mirror", _cameraMirror );
	_cameraGrabW	= ini.get( "Camera.grabW", _cameraGrabW );
	_cameraGrabH	= ini.get( "Camera.grabH", _cameraGrabH );
	_cameraProcessW = ini.get( "Camera.processW", _cameraProcessW );
	_cameraProcessH = ini.get( "Camera.processH", _cameraProcessH );
	_cameraOptFlowW = ini.get( "Camera.optFlowW", _cameraOptFlowW );
	_cameraOptFlowH = ini.get( "Camera.optFlowH", _cameraOptFlowH );


	_runDurationSec = ini.get( "Run.durationSec", _runDurationSec );


	//вычисления
	_w = _w * scale;
	_h = _h * scale;

	_renderAngle	= _rotate * 90;
	_renderW		= _w;
	_renderH		= _h;
	if ( ((_renderAngle + 360) / 90) % 2 == 1 ) { //поворот на 90, 270 градусов
		swap( _renderW, _renderH );
	}

}

//--------------------------------------------------------------
void RunInfo::setupInSetup()
{
	if ( _introDuration > 0 && _introImageFile != "" ) {
		_introImage.loadImage( _introImageFile );
	}
}
//--------------------------------------------------------------
void RunInfo::drawBegin( int factW, int factH )	//начало рисования - установить поворот
{
	ofPushMatrix();    //запомнить матрицу преобразования

	//вращение
	ofTranslate( screenWidth() / 2, screenHeight() / 2 );	//перенос
	ofRotate( renderAngle() );	//вращение
	ofTranslate( -renderWidth() / 2, -renderHeight() / 2 );	//перенос
	
	//растягивание
	if ( factW > 0 && factH > 0 ) {
		//расчет параметров вывода на экран
		float scaleOutX = 1.0 * renderWidth() / factW;
		float scaleOutY = 1.0 * renderHeight() / factH;
		
		/*if ( _resizeMode == ResizeModeCrop ) {
			scaleOut = max( 1.0 * renderWidth() / factW, 1.0 * renderHeight() / factH );
		}
		if ( _resizeMode == ResizeModeFit ) {
			scaleOut = min( 1.0 * renderWidth() / factW, 1.0 * renderHeight() / factH );
		}*/

		ofTranslate( renderWidth() / 2, renderHeight() / 2 );	//перенос
		ofScale( scaleOutX * ((_mirror == 0) ? 1.0 : -1.0), scaleOutY );
		ofTranslate( -factW / 2, -factH / 2 );	//перенос
	}
	


}

//--------------------------------------------------------------
void RunInfo::drawEnd()	//конец рисования - вернуть поворот
{
	//восстанавливаем матрицу преобразования
	ofPopMatrix();


	const float now = ofGetElapsedTimef();

	//показ картинки-intro
	const float introSec = introDuration();
	if ( introSec > 0.01 ) {
		if ( now < introSec ) {
			const float introFade = _fadeDuration;		//время затенения intro
			int alpha = 255;
			if ( now > introSec - introFade ) {
				alpha = 255 * (1.0 * (introSec - now ) / introFade ); 
			}
			ofEnableAlphaBlending();
			ofSetColor( 255, 255, 255, alpha );
			_introImage.draw( 0, 0,  renderWidth(), renderHeight() );
			ofDisableAlphaBlending();
		}
		
	}

	//если начало или конец - то плавно меняем яркость экрана
	const float fadeSec = _fadeDuration;
	if ( fadeSec > 0.01 ) {
		bool fade = false;
		float alpha = 0.0;
		if ( durationControl() && now >= _runDurationSec - fadeSec && _runDurationSec > 0.1 ) {
			fade = true;
			alpha = 1 - (_runDurationSec - now) / fadeSec;
		}
		if ( now < fadeSec ) {
			fade = true;
			alpha = 1 - now / fadeSec;
		}
		if ( fade ) {
			ofEnableAlphaBlending();
			ofFill();
			ofSetColor( 0, 0, 0, alpha * 255 );
			ofRect( 0, 0, renderWidth(), renderHeight() );

			ofNoFill();
			ofDisableAlphaBlending();
		}
	}

	//Выдача Fps на экран
	if ( printFps() ) {
		doPrintFps();
	}

	//восстанавливаем матрицу преобразования
	//ofPopMatrix();

	
}

//--------------------------------------------------------------
void RunInfo::updateBegin()
{
	_updateStart = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void RunInfo::update()	//если пора выйти - выходит
{
	float now = ofGetElapsedTimef();
	if ( durationControl() && now >= _runDurationSec && _runDurationSec > 0.1 ) {
		cout << "RunInfo: Exit by timer" << endl;
		OF_EXIT_APP(0);	
	}
	_updateEnd = now;
}

//--------------------------------------------------------------

void RunInfo::doPrintFps()	//выдать на экран fps
{
/*	float _lastElapsedTimeGrab = 0.0;		//для расчета dt при кадре с камеры
float _lastElapsedTimeUpdate = 0.0;	//для расчета dt при просто движении объектов
		float now = ofGetElapsedTimef();	//число секунд со старта приложения
		double dt = now - _lastElapsedTimeGrab;
		dt = min( dt, 0.1 );
		_lastElapsedTimeGrab = now;
*/

	ofSetColor( 0, 0, 0 );
	ofFill();
	ofRect( 0, 0, 350, 30 );
	ofSetColor(0xffffff);
	char reportStr[1024];
	sprintf(reportStr, "fps: %5d, update: %f ms", int(ofGetFrameRate()), _updateEnd - _updateStart );
	ofDrawBitmapString( reportStr, 3, 20 ); 
}