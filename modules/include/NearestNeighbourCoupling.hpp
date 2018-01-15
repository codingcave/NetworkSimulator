#ifndef NEARESTNEIGHBOURCOUPLING_H
#define NEARESTNEIGHBOURCOUPLING_H

class NearestNeighbourCouplingModule:
    public CouplingModule
    //public Registry
{
    public:
        NearestNeighbourCouplingModule();
        virtual ~NearestNeighbourCouplingModule();
        virtual const std::string getVersion() const override;
        virtual AbstractCoupling* getCoupling(chimera::vec_t_LuaItem& parameters) const override;
        virtual const std::string getGUID() const override;
    protected:
    private:
};

class NearestNeighbourCoupling
    :public TemplateCoupling
    <boost::numeric::ublas::vector<double> >
{
    public:
        NearestNeighbourCoupling(double sigma, int count);
        virtual ~NearestNeighbourCoupling();
        virtual void operator()(const boost::numeric::ublas::vector<boost::numeric::ublas::vector<double> >& system, int i, int j, boost::numeric::ublas::vector<double>& out) override;
    protected:
    private:
        double _sigma;
        int _count;
};

#endif // NEARESTNEIGHBOURCOUPLING_H
