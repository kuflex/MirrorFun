#include "Physics.h"
#include "ofMath.h"


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

RVertexProperties::RVertexProperties() {
	mass = //5.0;
			7.0;
		 
	elastCenter = 
						2; //2.0; //1.0;
	elastInside = 
						15;//15;
	elastOutside = 
						15;//15;
	friction = 2.0; //4.0;

	forceScale = 1.0;
	timeScale = 1.0;

	type = TypeMoving;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RVertex::updateNeighDist( const VertexList &vertices )	//вычислить расстояния до соседей
{
	int n = _neigh.size();
	_neighDist.resize( n );
	for ( int i = 0; i < n; i++ ) {
		_neighDist[ i ] = _pos.distance( vertices[ _neigh[ i ] ]._pos0 );
	}
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RVertex::updateCalculate( double dt, const RPoint &gForce, const VertexList &vertices )
{
	const double limitA = 1000.0;					//ПАРАМЕТР
	const double limitSpeed = 1000.0; //1.0;		//ПАРАМЕТР


	dt *= _prop.timeScale;							//учет возможного изменения масштаба времени

	_tempPos	= _pos;
	_tempSpeed	= _speed;
	if ( _prop.type == RVertexProperties::TypeMoving ) {

		//Геометрический сдвиг с учетом ограничений
		/*
		RPoint sum( 0, 0 );
		int sumN = 0;
		for (int i=0; i<_neigh.size(); i++) {
			const RVertex &vertex = vertices[ _neigh[ i ] ];
			RPoint delta = vertex._pos - _pos;

			double ideal = _neighDist[ i ];
			double d = delta.length() - ideal;	//различие в длинах

			//если различие большое - то вести себя как ограничитель
			double thresh = ideal * accept;			
			if ( fabs( d ) > thresh && delta.length() > 0 ) {	
				double rad = ( d > 0 ) ? ( d - thresh ) : ( d + thresh );
				RPoint q = rad * delta.getNormalized();
				
				sum += q;
				sumN++;
			}
		}
		if ( sumN > 0 ) {
			sum /= sumN;
			_tempPos += sum;
		}*/
		

		//Расчет сил
		RPoint force(0,0);

		//Сила Гука для центра
		{
			RPoint delta = _pos0 - _pos;
			double d = delta.length();
			d *= _prop.elastCenter;
			force += d * delta.normalize() * _size;

		}

		//Сила Гука для других частиц
		for (int i=0; i<_neigh.size(); i++) {
			const RVertex &vertex = vertices[ _neigh[ i ] ];
			RPoint delta = vertex._pos - _pos;
			double d = delta.length() - _neighDist[i];	//различие в длинах
			d *= ( d >= 0 ) ? _prop.elastOutside : _prop.elastInside;		//величина силы согласно закону Гука
			force += d  * delta.normalize() * _size;				//TODO второй раз считается длина
		}
		//сила тяжести
		force += gForce;

		force *= _prop.forceScale;
		//сила трения
		//ofPoint fric = force.normalize();
		//fric = fric * (-0.5);
		//force += fric;


		//расчет ускорения
		RPoint a = (1.0 / _area) * (1.0/_prop.mass) * force;

		a.limit( limitA );						//ограничение

		//расчет скорости
		_tempSpeed += dt * a;					//учет ускорения

		float fric = ( 1.0 - _prop.friction * dt );
		fric = max( fric, 0.0f );
		_tempSpeed *= fric;	//учет трения


		_tempSpeed.limit( limitSpeed );			//ограничение

		//расчет перемещения
		_tempPos += dt * _tempSpeed;

	}
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RVertex::updateApply()
{
	_pos	= _tempPos;
	_speed	= _tempSpeed;

}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RMesh::update( double dt, const RPoint &gForce )
{
	for (int i=0; i<v.size(); i++) {
		v[i].updateCalculate( dt, gForce, v );
	}
	for (int i=0; i<v.size(); i++) {
		v[i].updateApply();
	}
}

void RMesh::update( double dt, const vector<RPoint> &force ) //отдельные силы на вершины
{
	int n = 10; //5;
	float scale = 1.0 / n;
	for (int i=0; i<n; i++) {
		for (int i=0; i<v.size(); i++) {
			v[i].updateCalculate( dt * scale, force[i] /* * scale*/, v );
		}
		for (int i=0; i<v.size(); i++) {
			v[i].updateApply();
		}
	}
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RMesh::reset()
{
	for (int i=0; i<v.size(); i++) {
		v[i].reset();
	}
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RMesh::setup( int gridW, int gridH )
{
	vector<int> grid( gridW * gridH, 1 );
	setup( grid, gridW, gridH );
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void RMesh::setup( const vector<int> &grid, int gridW, int gridH )
{	

	RVertexProperties prop0;

	w = gridW;
	h = gridH;

	cell.clear();
	v.clear();
	int vW = gridW + 1;
	int vH = gridH + 1;
	vector<int> reg( vW * vH, -1 );

	_scalePos = max( vW, vH );	//чтоб сетка балы внутри квадрата [0,1]x[0,1]

	//регистрация вершин
	int count = 0;
	for (int y=0; y<gridH; y++) {
		for (int x=0; x<gridW; x++) {
			if ( grid[ x + gridW * y ] == 1 ) {
				for (int b = y; b<=y+1; b++) {
					for (int a = x; a<=x+1; a++) {
						if ( reg[ a + vW * b ] == -1 ) {
							reg[ a + vW * b ] = count;
							count++;
							RVertex _v;
							
							//свойства вершины
							RVertexProperties prop = prop0;
							prop.type = ( a==0 || b == 0 || a == vW -1 || b == vH - 1 ) 
												? RVertexProperties::TypeFixed : RVertexProperties::TypeMoving;
							float area = 1.0 / (gridW * gridH);

							_v.setup( RPoint( a / _scalePos, b / _scalePos ), prop, area );							
							v.push_back( _v );
						}

					}
				}				
			}
		}
	}

	//регистрация клеток
	for (int y=0; y<gridH; y++) {
		for (int x=0; x<gridW; x++) {
			if ( grid[ x + gridW * y ] == 1 ) {
				RCell _cell;			
				
				for (int i=0; i<4; i++) {
					int a = ( i == 0 || i == 1 ) ? 0 : 1;
					int b = ( i == 0 || i == 3 ) ? 0 : 1;
					a += x;
					b += y;
					_cell.ind.push_back( reg[ a + vW * b ] );
					_cell.p.push_back( RPoint( a, b ) );
				}
				cell.push_back( _cell );
			}
		}
	}


	//соседи
	for (int i=0; i<cell.size(); i++) {
		IndexList &ind = cell[i].ind;
		for ( int a = 0; a < ind.size() - 1; a++ ) {
			for ( int b = a + 1; b < ind.size(); b++ ) {
				v[ ind[a] ].addNeigh( ind[b] );
				v[ ind[b] ].addNeigh( ind[a] );
			}
		}
	}
	

	//завершение - расчет расстояний до соседей
	for (int i=0; i<v.size(); i++) {
		v[i].updateNeighDist( v );
	}

}


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------