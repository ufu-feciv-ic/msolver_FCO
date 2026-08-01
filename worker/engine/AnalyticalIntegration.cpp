#include "AnalyticalIntegration.h"

AnalyticalIntegration::AnalyticalIntegration()
{
    integrationVersion = NormativeIntegrationVersion::ABNT_NBR6118_2014;
}

void AnalyticalIntegration::setVersion(NormativeIntegrationVersion modelVersion)
{
    integrationVersion = modelVersion;
}

double AnalyticalIntegration::computeNormalConcreteRectangular(double multFcd, double fcd, double etaC, 
                                                               double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double ncd2 = multFcd * fcd * ((coef1 * coordY) + (coef2 * coordY * coordY / 2));

            return ncd2 * 1000; // kN
            break;
        }

        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double ncd2 = multFcd * etaC * fcd * ((coef1 * coordY) + (coef2 * coordY * coordY / 2));

            return ncd2 * 1000; // kN
            break;
        }

        default:
        {
            return 0;
        }
    }
}

double AnalyticalIntegration::computeNormalConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd,
                                                             double multFcd, double etaC, double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double g = coordLN / 100;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            //double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double ncd1 = -multFcd * fcd *
                        (-((xEpc2 * eexp * (coef1 * n2 + coef2 * (g + xEpc2 + nConc * coordY + coordY))) / (n1 * n2)) -
                        coef1 * coordY - (coef2 * coordY * coordY) / 2);

            return ncd1 * 1000; // kN
            break;
        }
        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double g = coordLN / 100;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            //double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double ncd1 = -multFcd * etaC * fcd * 
                        (-((xEpc2 * eexp * (coef1 * n2 + coef2 * (g + xEpc2 + nConc * coordY + coordY))) / (n1 * n2)) -
                        coef1 * coordY - (coef2 * coordY * coordY) / 2);

            return ncd1 * 1000; // kN
            break;
        }

        default:
        {
            return 0;
        }
    }
}

double AnalyticalIntegration::computeMomentXXConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double mcd2xx = multFcd * fcd * ((coef1 * coordY * coordY / 2) + (coef2 * coordY * coordY * coordY / 3));

            return mcd2xx * 1000; // kN.m
            break;
        }
        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double mcd2xx = multFcd * etaC * fcd * ((coef1 * coordY * coordY / 2) + (coef2 * coordY * coordY * coordY / 3));

            return mcd2xx * 1000; // kN.m
            break;
        }
        default:
        {
            return 0;
        }
    }
}

double AnalyticalIntegration::computeMomentXXConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd,
                                                             double multFcd, double etaC, double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double g = coordLN / 100;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double mcd1xx = (multFcd * fcd * (3 * coef1 * ((n1) * (n2) * (n3)*coordY * coordY + 2 * xEpc2 * eexp * 
            (((g) + xEpc2) * (n3) + (3 + 4 * nConc + nConc * nConc) * coordY)) + 2 * coef2 * 
            ((n1) * (n2) * (n3)*coordY * coordY * coordY + 3 * xEpc2 * eexp * 
            (2 * (g) * (g) + 2 * xEpc2 * xEpc2 + 2 * xEpc2 * (n1)*coordY + 
            (2 + 3 * nConc + nConc * nConc) * coordY * coordY + 2 * (g) * (2 * xEpc2 + (n1)*coordY))))) / 
            (6 * (n1) * (n2) * (n3));

            return mcd1xx * 1000;

            break;
        }
        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double g = coordLN / 100;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double mcd1xx = (multFcd * etaC * fcd * (3 * coef1 * ((n1) * (n2) * (n3)*coordY * coordY + 2 * xEpc2 * eexp * 
            (((g) + xEpc2) * (n3) + (3 + 4 * nConc + nConc * nConc) * coordY)) + 2 * coef2 * 
            ((n1) * (n2) * (n3)*coordY * coordY * coordY + 3 * xEpc2 * eexp * 
            (2 * (g) * (g) + 2 * xEpc2 * xEpc2 + 2 * xEpc2 * (n1)*coordY + 
            (2 + 3 * nConc + nConc * nConc) * coordY * coordY + 2 * (g) * (2 * xEpc2 + (n1)*coordY))))) / 
            (6 * (n1) * (n2) * (n3));

            return mcd1xx * 1000;
            break;
        }
        default:
        {
            return 0;
        }
    }
}

double AnalyticalIntegration::computeMomentYYConcreteRectangular(double multFcd, double fcd, double etaC, double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double mcd2yy = -multFcd * fcd * ((coef1 * coef1 * coordY) + (coef1 * coef2 * coordY * coordY) + (coef2 * coef2 * coordY * coordY * coordY / 3)) / 2;

            return mcd2yy * 1000; // kN.m
            break;
        }
        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double mcd2yy = -multFcd * etaC * fcd * ((coef1 * coef1 * coordY) + (coef1 * coef2 * coordY * coordY) + (coef2 * coef2 * coordY * coordY * coordY / 3)) / 2;

            return mcd2yy * 1000; // kN.m
            break;
        }
        default:
        {
            return 0;
        }
    }
}

double AnalyticalIntegration::computeMomentYYConcreteParabolic(double coordLN, double coordEpc2, double nConc, double fcd, double multFcd, double etaC, double coef1, double coef2, double coordY)
{
    switch (integrationVersion)
    {
        case NormativeIntegrationVersion::ABNT_NBR6118_2014:
        {
            double g = coordLN / 100.0;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100.0; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double mcd1yy = -(multFcd*fcd*(3*coef1*coef1*(xEpc2*
            (6+5*nConc+nConc*nConc)*eexp+
            (n1)*(n2)*(n3)*coordY)+
            3*coef1*coef2*((n1)*(n2)*(n3)*coordY*coordY+
            2*xEpc2*eexp*(((g)+xEpc2)*(3+nConc)+
            (3+4*nConc+nConc*nConc)*coordY))+
            coef2*coef2*((n1)*(n2)*(n3)*coordY*coordY*coordY+
            3*xEpc2*eexp*(2*(g)*(g)+
            2*xEpc2*xEpc2+2*xEpc2*(n1)*coordY+
            (2+3*nConc+nConc*nConc)*coordY*coordY+
            2*(g)*(2*xEpc2+(n1)*coordY)))))/
            (6*(n1)*(n2)*(n3));

            return mcd1yy * 1000; // kN.m

            break;
        }
        case NormativeIntegrationVersion::ABNT_NBR6118_2023:
        {
            double g = coordLN / 100.0;                   // m
            double xEpc2 = (coordEpc2 - coordLN) / 100.0; // m
            double n1 = nConc + 1;
            double n2 = nConc + 2;
            double n3 = nConc + 3;

            double base_eexp = (g + xEpc2 - coordY) / xEpc2;
            double eexp = 0;

            if (base_eexp <= 0)
            {
                eexp = 0; // Avoid negative exponent
            }
            else
            {
                eexp = pow(base_eexp, n1);
            }

            double mcd1yy = -(multFcd*etaC*fcd*(3*coef1*coef1*(xEpc2*
            (6+5*nConc+nConc*nConc)*eexp+
            (n1)*(n2)*(n3)*coordY)+
            3*coef1*coef2*((n1)*(n2)*(n3)*coordY*coordY+
            2*xEpc2*eexp*(((g)+xEpc2)*(3+nConc)+
            (3+4*nConc+nConc*nConc)*coordY))+
            coef2*coef2*((n1)*(n2)*(n3)*coordY*coordY*coordY+
            3*xEpc2*eexp*(2*(g)*(g)+
            2*xEpc2*xEpc2+2*xEpc2*(n1)*coordY+
            (2+3*nConc+nConc*nConc)*coordY*coordY+
            2*(g)*(2*xEpc2+(n1)*coordY)))))/
            (6*(n1)*(n2)*(n3));

            return mcd1yy * 1000; // kN.m
            break;
        }
        default:
        {
            return 0;
        }
    }
}