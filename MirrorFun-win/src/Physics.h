#pragma once

#include <vector>
using namespace std;
#include "ofTypes.h"

class RVertex;
class RCell;

typedef ofVec2f RPoint;
typedef vector<RPoint> RPointList;

typedef vector<int> IndexList;
typedef vector<RVertex> VertexList;
typedef vector<RCell> CellList;


//свойства сетки
class RVertexProperties {
public:
	double mass;
	double elastCenter;			//бегут к центру 
	double elastInside; 		//чем больше - тем сильнее влияние соседей
	double elastOutside;  			
	double friction; 		//чем больше - тем быстрее все останавливается
	double forceScale;		
	//влияние силы оптического потока. Некоторые ячейки могут не влиять на нее, 
	//и быть легкими - и просто летать от сил от других ячеек

	double timeScale;		//скорость времени

	typedef int Type;
	static const int TypeMoving		= 0;
	static const int TypeFixed		= 1;
	Type type;


	RVertexProperties();

};



class RVertex {
public:
	RVertexProperties _prop;

	RPoint _pos0;						//начальное положение
	RPoint _pos, _speed;	

	double _area;				//площадь частицы - доля относительно размера сетки
	double _size;				//sqrt( area )


	//Установка положения и других характеристик вершины
	void setup( const RPoint &pos, const RVertexProperties &prop, double area )	
	{
		_pos0 = pos;
		_prop = prop;
		_area = area;
		_size = sqrt( fabs( area ) );
		reset();
	}

	void updateNeighDist( const VertexList &vertices );	//вычислить расстояния до соседей

	//Установка соседей. Будет вычисляться расстояние до соседей, поэтому
	//предполагается, что к моменту вызова начальные координаты всех точек уже выставлены
	void addNeigh( int neigh ) 
	{
		//добавлять, тольеко если такой вершины еще нет
		for (int i=0; i<_neigh.size(); i++ ) {
			if ( neigh == _neigh[i] ) return;
		}	
		_neigh.push_back( neigh );
	}

	//void addCheckSegment( int indexA, int indexB ) {
	//	_checkSegment.push_back( indexA );
	//	_checkSegment.push_back( indexB );
	//}

	void reset() {
		_pos = _pos0;
		_speed.set( 0, 0 );
	}

	void updateCalculate( double dt, const RPoint &gForce, const VertexList &vertices );
	void updateApply();
private:
	RPoint _tempPos, _tempSpeed;			//для временного хранения, до применения updateApply

	IndexList _neigh;				//соседи, используются при пересчете физики в update
	vector<double> _neighDist;		//расстояния до соседей

	//IndexList _checkSegment;		//идут парами номера вершин, которые задают отрезки,
									//которые нельзя пересекать - это для предотврящения схлапываия сетки
	//vector<double> _checkSign;		//расстояние со знаком до контрольного отрезка

};

//клетка, состоит из индексов вершин и (текстурных) координат
class RCell {
public:
	IndexList ind;
	RPointList p; 
};




class RMesh {
public:
	VertexList v;			//вершины
	CellList cell;				//клетки
	int w, h;				//размеры сетки
	float _scalePos;		//масштаб из прямоугольника в [0,1]x[0,1] до [0..gridW]x[0..gridH]

	void setup( const vector<int> &grid, int gridW, int gridH );
	void setup( int gridW, int gridH );

	void reset();
	void update( double dt, const RPoint &gForce );	//сила тяжести
	void update( double dt, const vector<RPoint> &force ); //отдельные силы на вершины
	
};


