#pragma once

#include "Point.h"

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <stdexcept>

using std::invalid_argument;
using std::vector;

class Polygon
{
private:
	vector<Point> polygonVertices;
	Point geometricCenter;
	double maxY;
	double minY;
	double polygonHeight;
	double polygonArea;
	double staticMomentX;
	double staticMomentY;
	double numPoints;
	double angle;
	double inertiaX, inertiaY, inertiaXY;
	double inertiaX_cg, inertiaY_cg, inertiaXY_cg;

	friend class cereal::access;
	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(polygonVertices),
			CEREAL_NVP(geometricCenter),
			CEREAL_NVP(maxY),
			CEREAL_NVP(minY),
			CEREAL_NVP(polygonHeight),
			CEREAL_NVP(polygonArea),
			CEREAL_NVP(staticMomentX),
			CEREAL_NVP(staticMomentY),
			CEREAL_NVP(numPoints)		
		);
	}

public:
	Polygon();

	void setVertices(vector<Point> collectedVertices);
	void addVertice(double coordX, double coordY);
	void setAngle(double collectedAngle);
	void removeLastVertice();
    void clearPolygonVertices();
    void clearGeometricCenter();
    void computeMaxCoordY();
    void computeMinCoordY();
	void computeHeight();
	void computeArea();
	void computeStaticMomentX();
	void computeStaticMomentY();
	void computeCentroid();
	void translateToCentroid();
	void rotateAroundCentroid();
	void computeInertia();

	vector<Point> getPolygonVertices() const;
    Point getGeometricCenter() const;
    double getAngle() const;
	double getMaxY() const;
	double getMinY() const;
	double getPolygonHeight() const;
	double getPolygonArea() const;
	double getVet0X() const;
	double getVet0Y() const;
	double getInertiaX_cg() const;
	double getInertiaY_cg() const;
	double getInertiaXY_cg() const;
    void SetNumPoints(int numPointsInput);
    int GetNumPoints() const;
    void GetTableData(int index, float *x, float *y) const;
    void SetTableData(int index, float x, float y);
};