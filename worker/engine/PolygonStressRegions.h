#pragma once

#include "Point.h"
#include "Polygon.h"

class PolygonStressRegions
{
private:
    Polygon originalPolygon;
    Polygon compressedRegion;
    Polygon parabolicRegion;
    Polygon rectangularRegion;
    double neutralAxisHeight;
    double plasticHeight;
    double ruptureHeight;

public:
    PolygonStressRegions();

    void setOriginalPolygon(const Polygon &polygon);
    void setDeformationHeight(double xLN, double xEc2, double xEcpu);
    Point computeIntersection(Point firstPoint, Point secondPoint, double cutCoordY);
    Polygon regionAboveHeight(Polygon polygon, bool upperCut, double cutCoordY);
    Polygon regionBetweenHeights(Polygon polygon, double upperCut, double lowerCut);
    void generateStressRegions();
    void clearStressRegions();
    
    Polygon getOriginalPolygon() const;
    Polygon getCompressedRegion() const;
    Polygon getParabolicRegion() const;
    Polygon getRectangularRegion() const;
    double getNeutralAxisHeight() const;
    double getPlasticHeight() const;
    double getRuptureHeight() const;
};