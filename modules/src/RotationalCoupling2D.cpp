#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <stdlib.h>
#include <boost/numeric/ublas/vector.hpp>

#include "Naming.hpp"
#include "ExtensionNaming.hpp"
#include "StateSynchrony.hpp"
#include "interfaces/ILogger.hpp"
#include "LoggingSystem.hpp"
#include "ParameterValue.hpp"
#include "ParameterType.hpp"
#include "def.hpp"
#include "extendTypes.hpp"
#include "types/LuaFunctionWrapper.hpp"
#include "ParameterTypeSystem.hpp"
#include "ItemContainer.hpp"
#include "Module.hpp"
#include "EntryPoint.hpp"
#include "EntryPointSystem.hpp"
#include "ChimeraSystem.hpp"
#include "interfaces/IEventListener.hpp"
#include "interfaces/IEventListenerProvider.hpp"
//#include "event/Observer.hpp"
//#include "event/StateEventListener.hpp"
//#include "NotificationManager.hpp"
#include "Network/AbstractCoupling.hpp"
#include "Network/TemplateCoupling.hpp"
#include "Network/CouplingModule.hpp"
#include "RotationalCoupling2D.hpp"

extern "C"
{
    chimera::Module* LoadModule()
    {
        return new RotationalCoupling2DModule();
    }

    void UnloadModule(chimera::Module* reg)
    {
        delete (RotationalCoupling2DModule*)reg;
    }
}

RotationalCoupling2DModule::RotationalCoupling2DModule()
{
    //ctor
}

RotationalCoupling2DModule::~RotationalCoupling2DModule()
{
    //dtor
}

const std::string RotationalCoupling2DModule::getVersion() const
{
    return "1.0.0";
}

const std::string RotationalCoupling2DModule::getGUID() const
{
    return "Nearest-Neighbour coupling";
}

AbstractCoupling* RotationalCoupling2DModule::getCoupling(chimera::vec_t_LuaItem& parameters) const
{
    double epsilon;
    double sigma;
    double phi;
    chimera::ParameterValue pvMatrix;

    if(parameters.size() == 1 && parameters[0].getType() == chimera::systemtypes::PID_TABLE)
    {
        bool inSigma = false;
        bool inEpsilon = false;
        bool inPhi = false;
        bool inMatrix = false;

        const std::string vectorVectorRealMetaName = std::string(chimera::simulation::Naming::Type_Vector) + "#" + std::string(chimera::simulation::Naming::Type_Vector) + "#" + std::string(chimera::typenames::TYPE_NUMBER);
        size_t type = getChimeraSystem()->getTypeSystem()->getParameterID(vectorVectorRealMetaName);

        chimera::map_t_LuaItem* paramMap = (chimera::map_t_LuaItem*)parameters[0].getValue();
        for(auto p : *paramMap)
        {
            if(p.first == "sigma" && p.second.getType() == chimera::systemtypes::PID_NUMBER)
            {
                inSigma = true;
                sigma = p.second;
            }
            if(p.first == "epsilon" && p.second.getType() == chimera::systemtypes::PID_NUMBER)
            {
                inEpsilon = true;
                epsilon = p.second;
            }
            if(p.first == "phi" && p.second.getType() == chimera::systemtypes::PID_NUMBER)
            {
                inPhi = true;
                phi = p.second;
            }
            if(p.first == "G" && p.second.getType() == type)
            {
                inMatrix = true;
                pvMatrix = p.second;
            }
        }
        if(!(inSigma && inEpsilon && inPhi && inMatrix))
        {
            return nullptr;
        }
        return new RotationalCoupling2D(epsilon, sigma, phi, pvMatrix);
    }
    /*
    else if(parameters.size() >= 2 && parameters[0].getType() == chimera::systemtypes::PID_NUMBER && parameters[1].getType() == chimera::systemtypes::PID_NUMBER)
    {
        sigma = parameters[0];
        count = parameters[1];
        return new RotationalCoupling2D(sigma, count);
    }*/

    return nullptr;
}

RotationalCoupling2D::RotationalCoupling2D(double epsilon, double sigma, double phi, chimera::ParameterValue couplingValue):
    _epsilon(epsilon),
    _sigma(sigma),
    _phi(phi)
{
    _sinPhi = sin(phi);
    _cosPhi = cos(phi);
    _couplingValue = new chimera::ParameterValue(couplingValue);
    struct chimera::simulation::T_VectorDef* vd = (struct chimera::simulation::T_VectorDef*)couplingValue.getValue();
    _couplingMatrix = (boost::numeric::ublas::vector<boost::numeric::ublas::vector<double> >*)vd->value;
}

RotationalCoupling2D::~RotationalCoupling2D()
{

}

void RotationalCoupling2D::operator()(const boost::numeric::ublas::vector<boost::numeric::ublas::vector<double> >& system, const double& t, const int& i, boost::numeric::ublas::vector<double>& out)
{
    static int N;
    N = system.size();

    //int k = out.size();
    out[0] = 0;
    out[1] = 0;
    for(int j = 0; j < N; j++)
    {
        out[0] += (*_couplingMatrix)[i][j] * (_cosPhi * (system[j][0] - system[i][0]) + _sinPhi * (system[j][1] - system[i][1]));
        out[1] += (*_couplingMatrix)[i][j] * (_cosPhi * (system[j][1] - system[i][1]) - _sinPhi * (system[j][0] - system[i][0]));
    }
    out[0] = _sigma / _epsilon * out[0];
    out[1] = _sigma * out[1];
}