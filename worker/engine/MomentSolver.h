#pragma once

#include "Point.h"
#include "Polygon.h"
#include "Reinforcement.h"
#include "ConcreteProperties.h"
#include "SteelProperties.h"
#include "StrainDistribution.h"
#include "PolygonStressRegions.h"
#include "AnalyticalIntegration.h"
#include "InternalForces.h"

#include <iostream>
#include <functional>
#include <vector>

class MomentSolver
{
private:
    int maxIterations;
    int iterations;
    double tolerance;
    double axialForceSum;
    bool converged;

    Point Mrd;
    StrainDistribution strainResult;

public:
    MomentSolver();

    void solveEquilibrium(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double Nsd);

    double computeAxialForceResultant(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double strain1, double strain2, double Nsd);

    Point computeMomentResultant(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double strain1, double strain2, double Nsd);

    void setStrainDistribution(Polygon &polygon, ConcreteProperties &concrete, StrainDistribution &strainDistribution, 
double strain1, double strain2);

    void setStressRegions(Polygon &polygon, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions);

    void setInternalForces(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double Nsd);

    double findRootBrent(std::function<double(double)> func, double a, double b, double tol, int maxIter);

    Point testRegion1(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double Nsd);
    
    Point testRegion2(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double Nsd);
    
    Point testRegion3(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, 
StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
InternalForces &internalForces, double Nsd); 

    int getIterations() const;
    bool isConverged() const;
    Point getMoment() const;
    vector<Point> getEnvelope() const;
    double getTopFiberStrain() const;
    double getBottomFiberStrain() const;
};