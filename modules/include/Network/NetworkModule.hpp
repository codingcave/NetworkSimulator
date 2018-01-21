#ifndef NETWORKMODULE_H
#define NETWORKMODULE_H

class NetworkModule:
    public chimera::simulation::SystemDynamicModule
{
    public:
        NetworkModule();
        virtual ~NetworkModule();
        virtual const std::string getGUID() const override;
        virtual void* getInstance(chimera::vec_t_LuaItem& parameters) const override;
        virtual void destroyInstance(void * const instance) const override;
        virtual const std::string getVersion() const override;
        virtual void load(chimera::EntryPoint const * const entryPoint, void const * const params) override;
        virtual void unload() override;
    private:
        chimera::simulation::TemplateOdeSystem<double, boost::numeric::ublas::vector<double> >* getSystem(const chimera::ParameterValue& param) const;
        TemplateCoupling<boost::numeric::ublas::vector<double> >* getCoupling(const chimera::ParameterValue& param) const;
        CouplingEntryPoint* _ep;
        const chimera::EntryPoint* _callingEP;
};

#endif // NETWORKMODULE_H