#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>

#include "ParameterValue.hpp"
#include "ParameterType.hpp"
#include "def.hpp"
#include "interfaces/IEventListener.hpp"
#include "event/Observer.hpp"
#include "EntryPointBase/AbstractEventProvider.hpp"
#include "event/DefaultEventProvider.hpp"
#include "event/NotificationManager.hpp"
#include "EntryPointBase/AbstractSystemDynamic.hpp"
#include "EntryPointBase/AbstractIntegrator.hpp"
#include "EntryPointBase/AbstractTemporalIntegrator.hpp"
#include "simulation/TemporalStateEventProvider.hpp"
#include "simulation/AbstractSimulation.hpp"
#include "simulation/TemporalSimulation.hpp"

chimera::runtime::TemporalSimulation::TemporalSimulation(chimera::simulation::AbstractTemporalIntegrator* integrator)
{
    _running = false;
    _integrator = integrator;
    _onStep = new chimera::simulation::TemporalStateEventProvider(integrator);
    registerEvent("step", _onStep);

}

chimera::runtime::TemporalSimulation::~TemporalSimulation()
{
    delete _onStep;
    //dtor
}

bool chimera::runtime::TemporalSimulation::isRunning() const
{
    return _running;
}

bool chimera::runtime::TemporalSimulation::getYield() const
{
    return _yield;
}

void chimera::runtime::TemporalSimulation::setYield(bool value)
{
    _yield = value;
    if(value)
        _running = false;
}

bool chimera::runtime::TemporalSimulation::nextStep()
{
    _running = true;
    if(!_integrator->finished())
    {
        _integrator->nextStep();
        _onStep->trigger(this);
        return true;
    }
    else
    {
        _running = false;
        return false;
    }
}

chimera::simulation::AbstractIntegrator * chimera::runtime::TemporalSimulation::getIntegrator()
{
    return _integrator;
}
