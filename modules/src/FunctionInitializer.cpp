#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/numeric/ublas/vector.hpp>

#include "Naming.hpp"
#include "ExtensionNaming.hpp"
#include "StateSynchrony.hpp"
#include "interfaces/ILogger.hpp"
#include "LoggingSystem.hpp"
#include "ParameterValue.hpp"
#include "ParameterType.hpp"
#include "def.hpp"
#include "types/LuaFunctionWrapper.hpp"
#include "ParameterTypeSystem.hpp"
#include "ItemContainer.hpp"
#include "Module.hpp"
#include "EntryPoint.hpp"
#include "EntryPointSystem.hpp"
#include "ChimeraSystem.hpp"
#include "EntryPointBase/AbstractSystemDynamic.hpp"
#include "EntryPointBase/AbstractInitializer.hpp"
#include "EntryPointBase/AbstractRandom.hpp"
#include "EntryPointBase/InitializerModule.hpp"
#include "FunctionInitializer.hpp"

namespace ublas = boost::numeric::ublas;

extern "C"
{
    chimera::Module* LoadModule()
    {
        return new FunctionInitializerModule();
    }

    void UnloadModule(chimera::Module* module)
    {
        delete (FunctionInitializerModule*)module;
    }
}

FunctionInitializerModule::FunctionInitializerModule()
{

}

FunctionInitializerModule::~FunctionInitializerModule()
{

}

const std::string FunctionInitializerModule::getGUID() const
{
    return "Function Initializer";
}

void FunctionInitializerModule::destroyInstance(void * const instance) const
{
    delete (chimera::simulation::AbstractInitializer*)instance;
}

const std::string FunctionInitializerModule::getVersion() const
{
    return "1.0.0";
}

chimera::simulation::AbstractInitializer* FunctionInitializerModule::getInitializerInstance(chimera::vec_t_LuaItem& parameters) const
{
    if(parameters.size() == 1 && parameters[0].getType() == chimera::systemtypes::PID_TABLE)
    {
        chimera::map_t_LuaItem* paramMap = (chimera::map_t_LuaItem*)parameters[0].getValue();
        for(auto p : *paramMap)
        {
            if(p.first == "func" && p.second.getType() == chimera::systemtypes::PID_LUAFUNC)
            {
                return new FunctionInitializer(getChimeraSystem()->getTypeSystem(), p.second);
            }
        }
    }
    else if(parameters.size() == 1 && parameters[0].getType() == chimera::systemtypes::PID_LUAFUNC)
    {
        return new FunctionInitializer(getChimeraSystem()->getTypeSystem(), parameters[0]);
    }
    return nullptr;
}

FunctionInitializer::FunctionInitializer(chimera::ParameterTypeSystem* ps, const chimera::ParameterValue& functionValue)
{
    _ps = ps;
    _functionValue = new chimera::ParameterValue(functionValue);
}

FunctionInitializer::~FunctionInitializer()
{
    delete _functionValue;
}

void FunctionInitializer::initialize(chimera::simulation::AbstractSystemDynamic* system, void* state)
{
    static const size_t pid_vecReal = _ps->getParameterID(std::string(chimera::simulation::Naming::Type_Vector) + "#" + std::string(chimera::typenames::TYPE_NUMBER));
    static const size_t pid_vecvecReal = _ps->getParameterID(std::string(chimera::simulation::Naming::Type_Vector) + "#" + std::string(chimera::simulation::Naming::Type_Vector) + "#" + std::string(chimera::typenames::TYPE_NUMBER));

    std::unordered_map<std::string, size_t> features = system->getFeatures();
    auto stateType = features.find(chimera::simulation::Naming::Feature_state_type);
    if(stateType != features.end())
    {
        if(stateType->second == pid_vecReal)
        {
            boost::numeric::ublas::vector<double>* vState = (boost::numeric::ublas::vector<double>*)state;
            for(int i = 0; i < vState->size(); i ++)
            {
                chimera::ParameterValue next = (*_functionValue)(i);
                if(next.getType() == chimera::systemtypes::PID_NUMBER) {
                    (*vState)[i] = next;
                }
            }
        }
        else if(stateType->second == pid_vecvecReal)
        {
            boost::numeric::ublas::vector<boost::numeric::ublas::vector<double> >* vState = (boost::numeric::ublas::vector<boost::numeric::ublas::vector<double> >*)state;
            for(int i = 0; i < vState->size(); i++)
            {
                for(int j = 0; j < (*vState)[i].size(); j++)
                {
                    chimera::ParameterValue next = chimera::ParameterValue((*_functionValue)(i, j));
                    if(next.getType() == chimera::systemtypes::PID_NUMBER) {
                        (*vState)[i][j] = next;
                    }
                }
            }
        }
    }
}
