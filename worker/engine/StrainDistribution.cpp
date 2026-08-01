#include "StrainDistribution.h"

StrainDistribution::StrainDistribution()
{
    neutralAxisCoord = 0.0;
    plasticStrainCoord = 0.0;
    ruptureStrainCoord = 0.0;
    topFiberStrain = 0.0;
    bottomFiberStrain = 0.0;
}

void StrainDistribution::setStrain(double strain1, double strain2)
{
    topFiberStrain = strain1;
    bottomFiberStrain = strain2;
}

void StrainDistribution::computeStrainDistribution(double plasticStrain, double ruptureStrain, double polygonHeight)
{
    double k = polygonHeight / (bottomFiberStrain/1000 - topFiberStrain/1000);

    // double k = polygonHeight / (-topFiberStrain/1000 - bottomFiberStrain/1000);
    neutralAxisCoord = (-topFiberStrain/1000) * k;
    plasticStrainCoord = ((-plasticStrain/1000) - (topFiberStrain/1000)) * k;
    ruptureStrainCoord = ((-ruptureStrain/1000) - (topFiberStrain/1000)) * k;
}

double StrainDistribution::getNeutralAxisCoord() const
{
    return neutralAxisCoord;
}

double StrainDistribution::getPlasticStrainCoord() const
{
    return plasticStrainCoord;
}

double StrainDistribution::getRuptureStrainCoord() const
{
    return ruptureStrainCoord;
}

double StrainDistribution::getTopFiberStrain() const
{
    return topFiberStrain;
}

double StrainDistribution::getBottomFiberStrain() const
{
    return bottomFiberStrain;
}