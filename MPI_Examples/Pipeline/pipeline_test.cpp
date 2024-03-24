// A simple distributed pipeline example with two stages

#include <iostream>
#include "../../src/MPIPipelineManager.hpp"
#include <vector>
#include <numeric>
#include <sstream>
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;
namespace mpi = boost::mpi;

class Generator : public Node<string>
{
public:
    Generator(int num_tasks)
    {
        this->num_tasks = num_tasks;
    }

    string run(string task) override
    {
        if (curr == num_tasks)
        {
            std::cout << "Generator Done" << std::endl;
            return EOS;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        vector<int> v(curr, 1);
        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << v;
        return ss.str();
    }

private:
    int curr = 0;
    int num_tasks;
};

class Print : public Node<string>
{
public:
    string run(string task) override
    {
        vector<int> v;
        std::stringstream ss(task);
        boost::archive::text_iarchive ia(ss);
        ia >> v;
        std::cout << "Sum: " << std::accumulate(v.begin(), v.end(), 0) << std::endl;
        return string("");
    }
};

int main()
{
    mpi::environment env;
    mpi::communicator world;
    MPIPipelineManager *pipeline = new MPIPipelineManager(&env, &world);
    Generator *generator = new Generator(10);
    Print *print = new Print();

    pipeline->add_pipeline_node(generator);
    pipeline->add_pipeline_node(print);

    pipeline->run_until_finish();
    return 0;
}
