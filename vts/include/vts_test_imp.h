#ifndef VTS_TEST_IMP_H
#define VTS_TEST_IMP_H

//vts
#include "vts_result.h"

//Qt
#include <QObject>

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

namespace vts
{

class VTSTestImp : public QObject
{
    Q_OBJECT

    public :

        virtual bool initialize(const json& config) = 0;
        virtual bool load() = 0;
        virtual bool configure() = 0;
        virtual bool run() = 0;
        virtual bool analyze() = 0;
        virtual bool analyze_test() = 0;
        virtual bool finalize() = 0;

        virtual json get_results() = 0;

        int get_current_state()
        {
            return m_current_state;
        }
        int get_n_states()
        {
            return m_n_states;
        }

    protected :
        std::shared_ptr<spdlog::logger> log;


        int m_current_state;
        int m_n_states;

        void set_current_state(int s)
        {
            m_current_state = s;
        }
        void set_n_states(int s)
        {
            m_n_states = s;
        }

    signals :
        void finished();
        void signal_current_state(int, int);

}; // class VTSTestImp

} // namespace vts

#endif
