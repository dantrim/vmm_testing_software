#ifndef VTS_FILEMANAGER_H
#define VTS_FILEMANAGER_H

//ROOT
#include "TFile.h"
class TDirectory;
class TTree;

//std/stl
#include <string>
#include <memory>

//json
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//logging
namespace spdlog {
    class logger;
}

namespace vts
{

class FileManager
{
    public :
        FileManager(std::string vmm_serial_id, const json& output_config);
        virtual ~FileManager();

        const std::string& vmm_serial_number() const { return m_vmm_serial_id; }
        void set_current_test(std::string test_name) { m_current_test = test_name; }
        const std::string& current_test() const { return m_current_test; }
        const json& output_config() const { return m_output_config; }

        static int existing_files(std::string dirname, std::string filename); 
        static bool dir_exists(std::string dir);
        bool create_output();

        TFile* file() { return m_rfile; }
        bool test_dir_exists(std::string test_name = "");
        bool add_test_dir(std::string test_name = "");
        TDirectory* get_test_dir(std::string test_name = "");
        TDirectory* dir_has_dir(TDirectory* dir, std::string check);
        bool store(TObject* obj);



    private :
        std::shared_ptr<spdlog::logger> log;
        TFile* m_rfile;

        TDirectory* m_current_test_dir;
        TDirectory* m_current_hist_dir;
        TDirectory* m_current_tree_dir;

        std::string m_vmm_serial_id;
        std::string m_current_test;
        json m_output_config;

}; // class FileManager

} // namespace vts

#endif
