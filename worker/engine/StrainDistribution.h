#pragma once

class StrainDistribution
{
private:
    double neutralAxisCoord;
    double plasticStrainCoord;
    double ruptureStrainCoord;
    double topFiberStrain;
    double bottomFiberStrain;

public:
    StrainDistribution();

    void setStrain(double strain1, double strain2);
    void computeStrainDistribution(double plasticStrain, double ruptureStrain, double polygonHeight);

    double getNeutralAxisCoord() const;
    double getPlasticStrainCoord() const;
    double getRuptureStrainCoord() const;
    double getTopFiberStrain() const;
    double getBottomFiberStrain() const;
};