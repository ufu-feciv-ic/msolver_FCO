#pragma once

#include "Point.h"
#include "Polygon.h"
#include "Reinforcement.h"
#include "ConcreteProperties.h"
#include "SteelProperties.h"
#include "StrainDistribution.h"
#include "PolygonStressRegions.h"
#include "AnalyticalIntegration.h"

class InternalForces
{
private:
    double normalConcrete;
    double momentUUConcrete;
    double momentVVConcrete;
    double normalSteel;
    double momentUUSteel;
    double momentVVSteel;
    double momentUUSection;
    double momentVVSection;
    double momentXXSection;
    double momentYYSection;
    double maxNormalCompression;
    double maxNormalTraction;
    double normalSolicitation;

public:
    InternalForces();

    void setNormalSolicitation(double Nsd);
    void computeNormalConcrete(AnalyticalIntegration &analyticalIntegration, ConcreteProperties &concrete,
                               PolygonStressRegions &stressRegions, StrainDistribution &strainDistribution);
    void computeMomentUUConcrete(AnalyticalIntegration &analyticalIntegration, ConcreteProperties &concrete,
                               PolygonStressRegions &stressRegions, StrainDistribution &strainDistribution);
    void computeMomentVVConcrete(AnalyticalIntegration &analyticalIntegration, ConcreteProperties &concrete, 
                                PolygonStressRegions &stressRegions, StrainDistribution &strainDistribution);
    void computeNormalSteel(Polygon &polygon, Reinforcement &reinforcement, SteelProperties &steel,
                            StrainDistribution &strainDistribution);
    void computeMomentUUSteel(Polygon &polygon, Reinforcement &reinforcement, SteelProperties &steel, 
                            StrainDistribution &strainDistribution);
    void computeMomentVVSteel(Polygon &polygon, Reinforcement &reinforcement, SteelProperties &steel, 
                            StrainDistribution &strainDistribution);
    void computeMomentUUSection();
    void computeMomentVVSection();
    void computeMomentXXSection(double angle);
    void computeMomentYYSection(double angle);
    void computeMaxCompression(Polygon &polygon, Reinforcement &reinforcement, SteelProperties &steel, 
                               ConcreteProperties &concrete);
    void computeMaxTraction(Polygon &polygon, Reinforcement &reinforcement, SteelProperties &steel);

    double getNormalSection() const;
    double getMomentXXSection() const;
    double getMomentYYSection() const;
    double getNormalConcrete() const;
    double getMomentConcrete() const;
    double getNormalSteel() const;
    double getMomentSteel() const;
    double getMaxNormalCompression() const;
    double getMaxNormalTraction() const;
    double getNormalSolicitation() const;
};