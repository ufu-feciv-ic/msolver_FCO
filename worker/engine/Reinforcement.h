#pragma once

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <vector>

#include "Point.h"

using std::vector;

class Reinforcement
{
private:
	vector<Point> reinforcement;
	vector<double> diameters;
	vector<double> areas;
	double effectiveDepth;
	double totalArea;

	friend class cereal::access;
	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(reinforcement),
			CEREAL_NVP(diameters),
			CEREAL_NVP(areas)
		);
	}

public:
	Reinforcement();

	void setReinforcement(vector<Point> collectedReinforcement, vector<double> collectedDiameters);
	void addReinforcement(double coordX, double coordY, double diameter);
	void removeLastReinforcement();
	void clearReinforcement();
	void computeArea();
	void translateToCentroidPolygon(Point centroid);
	void rotateAroundCentroidPolygon(double angle);
	void setEffectiveDepth(double maxY);

    vector<Point> getReinforcement() const;
	vector<double> getDiameters() const;
	vector<double> getAreas() const;
	double getEffectiveDepth() const;
    double getTotalArea() const;
    void SetNumPoints(int numPointsInput);
    int GetNumPoints() const;
    void GetTableData(int index, double *x, double *y, double *d) const;
    void SetTableData(int index, double x, double y, double d);
	void addNewDefaultReinforcement(double defaultDiameter);
};