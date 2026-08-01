#pragma once

#include "Point.h"
#include "Polygon.h"
#include "Reinforcement.h"
#include "ConcreteProperties.h"
#include "SteelProperties.h"
#include "StrainDistribution.h"
#include "PolygonStressRegions.h"

enum class NormativeIntegrationVersion
{
    ABNT_NBR6118_2014,
    ABNT_NBR6118_2023
};

class AnalyticalIntegration
{
private:
    NormativeIntegrationVersion integrationVersion;

public:
    AnalyticalIntegration();

    void setVersion(NormativeIntegrationVersion modelVersion);
    double computeNormalConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY);
    double computeNormalConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd, 
                                          double multFcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd, 
                                          double multFcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentXXConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentXXConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd, 
                                          double multFcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentYYConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY);
    double computeMomentYYConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd, 
                                          double multFcd, double etaC, double coef1, double coef2, double coordY);
};