#include "testApp.h"

#include "ofxOpenCv.h"
#include "MotionDetector.h"
#include "Physics.h"
#include "RunInfo.h"
#include "pbSounds.h"

string _imageFileName;
string _movieFileName;


//экран
bool testMode	= false;			//если true, то рисуется сетка   кнопка ' '

int w = 800; 
int h = 450; 


//камера
int grabW = 320;	
int grabH = 240;
int procW = 320;
int procH = 240;

ofVideoGrabber 			vidGrabber;
ofxCvColorImage			colorImg;
ofxCvColorImage			colorImgProc;
ofxCvGrayscaleImage		grayImg;
MotionDetector _motion;



//Физика
RMesh _mesh;
int _gridW, _gridH;
vector<RPoint> _force;

const float physTimeSpeedUp = 1.0;			//на сколько ускорить течение времени в моделировании физики

//картинка
bool _useTexture = false;
ofTexture _texture;
ofImage imageStatic;


//--------------------------------------------------------------
void testApp::setup(){
	ofSetWindowTitle("MirrorFun 4.2    Press Space to switch modes");

	runInfo.setup();	//он запустит окно OpenGL


	runInfo.setupInSetup();

	//ofSetFrameRate( 60 );
	ofSetVerticalSync( true );	//for smooth animation

	//ofBackground(255,255,255);
	ofBackground(0,0,0);

	//Камера
	grabW = runInfo.cameraGrabW();
	grabH = runInfo.cameraGrabH();
	procW = runInfo.cameraProcessW();
	procH = runInfo.cameraProcessH();
	vidGrabber.setVerbose(true);
	vidGrabber.setDeviceID( runInfo.cameraDevice() );
	vidGrabber.initGrabber( grabW, grabH );
	vidGrabber.setDesiredFrameRate( 30 );		//ПАРАМЕТР

	colorImg.allocate( grabW, grabH );
	colorImgProc.allocate( procW, procH );
	grayImg.allocate( grabW, grabH );


	//Физика
	_motion.setup( procW, procH );

	_gridW = 16; //32;
	_gridH = _gridW * ofGetHeight() / ofGetWidth();
	if ( _gridH > _gridW ) {		//случай экрана Portrait
		_gridH = _gridW;
		_gridW = _gridH * ofGetWidth() / ofGetHeight();
	}
	_mesh.setup( _gridW, _gridH );

	//_mesh.v[ _gridW/2 + _gridW * _gridH / 2 ]._pos += 0.5;


	int n = _mesh.v.size();
	_force.resize( n );	//выделяем память под силу



	//загрузка картинки

	_imageFileName = "imageOver.png";
	_useTexture = ( _imageFileName != "" );
	if ( _useTexture ) {
		ofImage img;
		img.loadImage( _imageFileName );
		if ( img.width == 0 ) {
			_useTexture = false;
		}
		else {
			int textureW = img.width;
			int textureH = img.height;
			int type = 0;
			if ( img.type == OF_IMAGE_COLOR ) type = GL_RGB;
			if ( img.type == OF_IMAGE_COLOR_ALPHA ) type = GL_RGBA;
			_texture.allocate(textureW, textureH, type, true);
			_texture.loadData(img.getPixels(), textureW, textureH, type);
		}
	}

	imageStatic.loadImage( "imageStatic.png" );


	testMode = true;



	runInfo.setupInSetup();

	ofHideCursor();		//скрыть мышь

}


//--------------------------------------------------------------

float _lastElapsedTimeGrab = 0;
float _lastElapsedTimeUpdate = 0;
float _lastResetTimef = -1;


void testApp::update(){
	runInfo.updateBegin();

	//double dt;
	//dt = 1.0 / 25 * physTimeSpeedUp;		

	bool bNewFrame = false;

	vidGrabber.update();
	if ( vidGrabber.isFrameNew() ){
		//расчет dt
		float now = ofGetElapsedTimef();	//число секунд со старта приложения
		double dt = now - _lastElapsedTimeGrab;
		dt = min( dt, 0.1 );
		_lastElapsedTimeGrab = now;

		//считывание картинки
		colorImg.setFromPixels( vidGrabber.getPixels(), grabW, grabH );
		if ( runInfo.cameraMirror() ) {			//учет отражения камеры
			colorImg.mirror( false, true );
		}
		colorImgProc.scaleIntoMe( colorImg );
		grayImg = colorImg;

		//Motion detector
		_motion.update( colorImgProc );	
	}


	//Вне камеры ----------------------------------------
	//расчет dt
	float now = ofGetElapsedTimef();	//число секунд со старта приложения
	double dt = now - _lastElapsedTimeUpdate;
	dt = min( dt, 0.1 );
	_lastElapsedTimeUpdate = now;

	{

		//Mesh
		int n = _mesh.v.size();
		vector<RPoint> &force = _force;
		//force[ 35 ] = ofPoint( 0.1, 0 );	//TEST


		const float motionThresh = 2.0; //5.0; //5.0;	//Порог учета движения
		const int motionRad = 5;		//Радиус анализа,   должен зависеть от разрешея камеры
		const float forceScale = 8.0;	//параметр усиления силы

		//считаем по вершинам.а точнее - считать по клеткам
		int mw = _motion.outWidth();
		int mh = _motion.outHeight();

		float scalePos = _mesh._scalePos;

		//масштабирование камеры		//TODO занести в детектор движения??
		float scaleCam = min( 1.0 / procW, 1.0 / procH );


		for ( int i=0; i<n; i++ ) {
			ofPoint p = _mesh.v[i]._pos;
			int mx = int(p.x * scalePos) * mw / _gridW;
			int my = int(p.y * scalePos) * mh / _gridH;
			int count = 0;
			force[i] = ofPoint( 0, 0 );
			int r = motionRad;
			for (int y1 = my - r; y1 <= my + r; y1++) {
				for (int x1 = mx - r; x1 <= mx + r; x1++) {
					if ( x1 >= 0 && x1 < mw && y1 >= 0 && y1 < mh ) {
						RPoint delta = *_motion.outData( x1, y1 );
						if ( delta.length() > motionThresh ) {		//TODO не инвариантно к размеру камеры
							ofPoint frc = delta * forceScale * scaleCam;
							force[ i ] += frc;
							count++;
						}
					}
				}
			}
			if ( count > 0 ) {
				force[ i ] /= count;
			}

		}
		_mesh.update( dt, force ); //отдельные силы на вершины
	}


	runInfo.update();
}

//--------------------------------------------------------------

void testApp::draw(){
	runInfo.drawBegin( w, h );							//поворот экрана

	ofEnableAlphaBlending();

	int scrW = w;
	int scrH = h;

	float offsetX = 0;
	float scaleX = 1.0;
	float scaleY = 1.0;


	float scalePos = _mesh._scalePos;
	float gridScaleX = 1.0 * scrW / _gridW * scalePos;
	float gridScaleY = 1.0 * scrH / _gridH * scalePos;

	ofSetColor(0xffffff);

	//камера
	if ( testMode ) {
		//vidGrabber.draw( 0, 0, scrW, scrH );
		colorImg.draw( 0, 0, scrW, scrH );
	}

	//texture.draw( 0, 0 );
	//Постер
	if ( !testMode )
	{


		//------------
		//if ( !_useTexture ) {
			colorImg.draw( -100, -100, 1, 1 );	//Фиктивное рисование, для обновления текстуры.
											//TODO если будет медленно работать - выкручиваться с vidGrabber
		//}

		//Сдвиг системы координат
		ofPushMatrix();
		ofTranslate( offsetX, 0.0 );


		int passes = ( _useTexture ) ? 2 : 1;
		for (int pass = 0; pass < passes; pass++) {
			ofTexture &tex = (pass == 0) ? colorImg.getTextureReference():_texture;
			tex.bind();				

			CellList &cell = _mesh.cell;

			//------------
			//медленно
			/*
			for ( int i=0; i<cell.size(); i++ ) {
			glBegin( GL_QUADS );						//TODO!!! оптимизировать - оним массивом
			IndexList &ind = cell[i].ind;
			for (int j = 0; j < ind.size() + 1; j++) {
			RVertex v = _mesh.v[ ind[ j % ind.size() ] ];
			RPoint p = v._pos;
			RPoint t = cell[i].p[ j % ind.size() ];
			glTexCoord2f( t.x * tex.texData.tex_t / _gridW, t.y * tex.texData.tex_u / _gridH ); 

			glVertex2f( p.x * gridScaleX, p.y * gridScaleY );
			}
			glEnd();
			}
			*/

			//быстро

			vector<float> pointP;
			vector<float> textureP;

			int n = cell.size();
			for ( int i=0; i<cell.size(); i++ ) {
				IndexList &ind = cell[i].ind;
				for (int j = 0; j < ind.size(); j++) { 
					RVertex v = _mesh.v[ ind[ j % ind.size() ] ];
					RPoint p = v._pos;
					RPoint t = cell[i].p[ j % ind.size() ];

					RPoint p_( p.x * gridScaleX, p.y * gridScaleY );
					RPoint t_( t.x * tex.texData.tex_t / _gridW, t.y * tex.texData.tex_u / _gridH );

					pointP.push_back( p_.x );
					pointP.push_back( p_.y );

					textureP.push_back( t_.x );
					textureP.push_back( t_.y );
				}
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, &pointP[0]);
			glTexCoordPointer( 2, GL_FLOAT, 0, &textureP[0] );
			glDrawArrays(GL_QUADS, 0, n * 4);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			//------

			tex.unbind();
		}


		//Статичное изображение
		if ( imageStatic.width > 0 ) {
			ofSetColor( 255, 255, 255 );
			imageStatic.draw( 0, 0, scrW, scrH );
		}

		/*if ( alpha < 255 ) {
		ofDisableAlphaBlending();
		}*/



		//возврат
		ofPopMatrix();
	}

	//motion flow
	/*	
	ofSetColor( 255, 0, 0 );
	int scale = scrW / _motion.outWidth();
	for (int y = 0; y < _motion.outHeight(); y++) {
	for (int x = 0; x < _motion.outWidth(); x++ ) {
	ofPoint *d = _motion.outData( x, y );
	ofCircle( x * scale, y * scale, 1 );
	ofLine( x * scale, y * scale, (x + d->x) * scale, (y + d->y) * scale );

	}
	}
	*/


	//Mesh
	if ( testMode )
	{
		//ofSetColor( 0, 0, 0 );

		/*for (int pass = 0; pass < 2; pass++ ){		
		{
		if ( pass == 0 ) {
		ofSetColor( 0, 0, 0 );
		ofSetLineWidth( 5 );
		}
		else {
		ofSetColor( 255, 0, 255 );
		ofSetLineWidth( 1 );
		}*/
		ofSetColor( 255, 0, 255 );
		ofSetLineWidth( 3 );


		CellList &cell = _mesh.cell;
		for ( int i=0; i<cell.size(); i++ ) {			
			IndexList &ind = cell[i].ind;
			for (int j = 0; j < ind.size(); j++) {
				RVertex v1 = _mesh.v[ ind[ j % ind.size() ] ];
				RVertex v2 = _mesh.v[ ind[ (j + 1) % ind.size() ] ];
				ofLine( v1._pos.x * gridScaleX, v1._pos.y * gridScaleY, 
					v2._pos.x * gridScaleX, v2._pos.y * gridScaleY );

			}
		}
		//}
	}
	
	ofDisableAlphaBlending();

	runInfo.drawEnd();						//возврат из поворота экрана
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		case ' ':
			testMode = !testMode;
			break;
		case 't': runInfo.setPrintFps( !runInfo.printFps() );	//вкл-выкл выдачу fps на экран
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	//if ( button == 0 ) {
	//	OF_EXIT_APP( 0 );
	//}
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

