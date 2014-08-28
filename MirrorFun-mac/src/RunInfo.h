#pragma once

#include "ofMain.h"



typedef int pbScreenFormat;	//формат экрана
const pbScreenFormat pbScreenFormat_4x3		= 0;
const pbScreenFormat pbScreenFormat_16x9	= 1;
const pbScreenFormat pbScreenFormat_16x10	= 2;

string screenFormatToStr( pbScreenFormat format );
pbScreenFormat screenFormatFromStr( const string &str );


class RunInfo
{
public:
	RunInfo();
	//вызывать в setup()
	//если window = 0, то окно не создает
	void setup();	//на самом деле, это startup, в нем нельзя грузить картинки
	void setupInSetup();			//вызывать в обычном setup - загрузка картинок
	
	
	//вызывать в update()
	void updateBegin();
	void update();		//если пора выйти - выходит

	//вызывать в draw()
	void drawBegin( int factW = 0, int factH = 0 );	//начало рисования - установить масштаб и поворот экрана 
	//factW, factH - если не 0, то это размеры экрана, в которые приложение хочет рисовать
	//в этом случае runInfo осуществляет масштабирование и отсечение

	void drawEnd();	//конец рисования - вернуть поворот

	void setPrintFps( bool enable ) { _printFps = enable; }	//вкл-выкл выдачу fps на экран
	bool  printFps()			{ return _printFps; }

	//вспомогательные параметры
	int fullScreen() { return _fullScreen; }
	int screenWidth()	{ return _w; }
	int screenHeight()	{ return _h; }

	int renderWidth()	{ return _renderW; }
	int renderHeight()	{ return _renderH; }
	int renderAngle()	{ return _renderAngle; }

	//формат экрана
	pbScreenFormat screenFormat() { return _screenFormat; }

	//Звук
	float soundVolume() { return _soundVolume; }	//масштаб изменения громкости звука, 0..1

	//Камера
	int cameraDevice()	{ return _cameraDevice;	}	//номер используемой камеры
	int cameraDevice2()	{ return _cameraDevice2;	}	//номер используемой камеры 2
	int cameraFrameRate()	{ return _cameraFrameRate; }
	int cameraMirror()	{ return _cameraMirror; }	//отражена ли камера
	int cameraGrabW()	{ return _cameraGrabW; }	//захват картинки
	int cameraGrabH()	{ return _cameraGrabH; }
	int cameraProcessW()	{ return _cameraProcessW; }	//обработка картинки, трудозатратные процедуры
	int cameraProcessH()	{ return _cameraProcessH; }
	int cameraOptFlowW()	{ return _cameraOptFlowW; }	//обработка картинки, оптический поток
	int cameraOptFlowH()	{ return _cameraOptFlowH; }

	float fadeDuration()	{ return _fadeDuration;	}	//фейдинг экрана из темноты при старте-конце программы

	float introDuration()	{ return _introDuration; }	//длительность показа стартовой картинки
	string introImageFile()		{ return _introImageFile; }		//стартовая картинка

	bool durationControl()	{ return _durationControl; }	//контролировать ли продолжительность времени работы
	void setDurationControl( bool durationControl )	{	_durationControl = durationControl; }

	static const int ResizeModeCrop = 0;
	static const int ResizeModeFit = 1;

private:
	int _w;
	int _h;
	int _rotate;		//разворот экрана
	int _fullScreen;		//на весь экран
	int _mirror;		//зеркальное отражение
	float	_fadeDuration;	//длительность затемнения при старта-конце программы

	float _introDuration; 	//длительность показа стартовой картинки
	string _introImageFile;		//стартовая картинка - имя файла
	ofImage _introImage;		//стартовая картинка


	int _resizeMode;	//0 - crop, 1 - fit

	int _renderW;
	int _renderH;
	int _renderAngle;
	pbScreenFormat _screenFormat;

	float _runDurationSec;		//желаемое время работы приложения, если 0 - то не останавливается

	//Звук
	float _soundVolume;	

	//Камера
	int _cameraDevice;
	int _cameraDevice2;
	int _cameraFrameRate;
	int _cameraMirror;
	int _cameraGrabW;
	int _cameraGrabH;
	int _cameraProcessW;
	int _cameraProcessH;
	int _cameraOptFlowW;
	int _cameraOptFlowH;

	//
	bool _durationControl;

	bool _printFps;
	void doPrintFps();	//выдать на экран fps

	float _updateStart;
	float _updateEnd;
};

extern RunInfo runInfo;



