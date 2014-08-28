#include "pbSounds.h"


pbSounds sharedSounds;

//----------------------------------------------------
void pbSound::load( ofxIniSettings &ini, const string &path, float globalVolume, const string &dir )
{
	name	= ini.get( path + "Name", string() );
	volume  = ini.get( path + "Volume", 1.0f );
	string fileName = dir + ini.get( path + "Sample", string() );
	if ( fileName != "" ) {
		sample.loadSound( fileName );
		sample.setVolume( globalVolume * volume );
		sample.setMultiPlay( true );
	}
}

//----------------------------------------------------
void pbSounds::setup( const string &fileName, float globalVolume )
{
	ofxIniSettings ini;
	ini.load( fileName );

	string dir = ini.get( "Sounds.contentDir", string() );
	if ( dir != "" ) dir += "/";

	_globalVolume = globalVolume;
	int N = ini.get( "Sounds.N", 0 );
	_sound.resize( N );
	for (int i=0; i<N; i++) {
		_sound[i].load( ini, "Sound" + ofToString( i + 1 ) + ".", globalVolume, dir );
	}

}

//----------------------------------------------------
void pbSounds::update()
{
	ofSoundUpdate();
}

//----------------------------------------------------
void pbSounds::play( const string &name, float speed )
{
	int i = find( name );
	if ( i >= 0 ) {
		_sound[i].sample.play();
		_sound[i].sample.setSpeed( speed );
	}
	else {
		cout << "WARNING: No sound with name: " + name << endl;
	}
}


//----------------------------------------------------
int pbSounds::find( const string &name )
{
	for ( int i=0; i<_sound.size(); i++) {
		if ( _sound[i].name == name ) return i;
	}
	return -1;
}

//----------------------------------------------------
