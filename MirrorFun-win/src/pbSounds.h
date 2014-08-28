#pragma once

#include "ofMain.h"
#include "ofxIniSettings.h"

//Звуковой движок

struct pbSound {
	string name;
	float volume;

	ofSoundPlayer sample;
	void load( ofxIniSettings &ini, const string &path, float globalVolume, const string &dir );

};




class pbSounds
{
public:
	void setup( const string &fileName, float globalVolume );
	void update();
	void play( const string &name, float speed = 1.0f );
	
	vector<pbSound> &sound() { return _sound; }
private:
	vector<pbSound> _sound;
	float _globalVolume;

	int find( const string &name );

};

extern pbSounds sharedSounds;