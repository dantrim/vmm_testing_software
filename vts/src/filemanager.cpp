//vts
#include "filemanager.h"
#include "helpers.h"

//std/stl
#include <sstream>
#include <iostream>
using namespace std;

//logging
#include "spdlog/spdlog.h"

//Qt
#include <QStringList>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFile>
#include <QDir>

//ROOT
//#include "TIter.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TKey.h"
#include "TDirectory.h"

namespace vts
{

FileManager::FileManager(std::string vmm_serial_id, const json& output_config)
{
    log = spdlog::get("vts_logger");
    log->debug("{0} - VMM serial {1}, Received output_config {2}",__VTFUNC__,vmm_serial_id, output_config.dump());
    m_file_ext = 0;
    m_output_directory = "";

    m_vmm_serial_id = vmm_serial_id;
    m_output_config = output_config;
    m_current_test_dir = nullptr;
    m_current_hist_dir = nullptr;
    m_current_tree_dir = nullptr;
}


FileManager::~FileManager()
{
    if(m_rfile != nullptr)
    {
        m_rfile->Write();
        m_rfile->Close();
        delete m_rfile;
        m_rfile = nullptr;
    }
}

bool FileManager::dir_exists(string dir)
{
    QDir qdir(QString::fromStdString(dir));
    return qdir.exists();
}

bool FileManager::create_output()
{
    stringstream outdir;
    std::string output_dir = output_config().at("output_directory").get<std::string>();
    outdir << output_dir;

    QString qdir = QString::fromStdString(output_dir);
    if(!qdir.endsWith("/")) outdir << "/";
    outdir << "VTS_VMM_" << m_vmm_serial_id;
    outdir << "/";

    if(!dir_exists(outdir.str()))
    {
        // create
        log->info("{0} - Creating test output directory: {1}",__VTFUNC__,outdir.str());
        QDir().mkdir(QString::fromStdString(outdir.str()));
        if(!dir_exists(outdir.str()))
        {
            log->error("{0} - Test output directory (={1}) does not exist, failed to create it!",__VTFUNC__,outdir.str());
            m_output_directory = "";
            return false;
        }
    }

    m_output_directory = outdir.str();

    stringstream filename_no_ext;
    filename_no_ext << "vts_data_VMM";
    filename_no_ext << m_vmm_serial_id;
    int ext_no = existing_files(outdir.str(), filename_no_ext.str());
    m_file_ext = ext_no;
    bool need_to_add_extension = (ext_no>0);

    stringstream outputfile;
    outputfile << outdir.str();
    outputfile << "vts_data_VMM";
    outputfile << m_vmm_serial_id;

    if(need_to_add_extension)
    {
        stringstream of;
        of << filename_no_ext.str() << "_%04d.root";
        QString filename_init = QString::fromStdString(of.str());
        const char* filename_formed = Form(filename_init.toStdString().c_str(), ext_no);
        string filename_formed_str(filename_formed);
        outputfile.str("");
        outputfile << outdir.str();
        outputfile << filename_formed_str;
    }
    else
    {
        outputfile << ".root";
    }

    stringstream testfile;
    testfile << outputfile.str();

    log->info("{0} - Output file for test data: {1}",__VTFUNC__,testfile.str());

    if(QFile::exists(QString::fromStdString(testfile.str())))
    {
        log->error("{0} - Output test file already exists: {1}, using this one",__VTFUNC__,testfile.str());
        m_rfile = TFile::Open(testfile.str().c_str(), "UPDATE");
        if(m_rfile->IsZombie())
        {
            log->error("{0} - Output file is zombie!",__VTFUNC__);
            return false;
        }
    }
    else
    {
        m_rfile = new TFile(testfile.str().c_str(), "UPDATE");
        if(m_rfile->IsZombie())
        {
            log->error("{0} - Output file could not be created, it is a zombie!",__VTFUNC__);
            return false;
        }
    }

    return true;
}

int FileManager::existing_files(string dirname, string filename_no_ext)
{
    std::shared_ptr<spdlog::logger> slog;
    slog = spdlog::get("vts_logger");
    bool ok;
    QStringList filters;
    stringstream filter;
    filter << filename_no_ext << "*.root";
    filters << QString::fromStdString(filter.str());
    QDir qdir(QString::fromStdString(dirname));
    qdir.setNameFilters(filters);

    int max_ext_found = -1;
    QFileInfoList listOfFiles = qdir.entryInfoList();

    if(listOfFiles.size()>0)
    {
        for(int i = 0; i < listOfFiles.size(); i++)
        {
            QFileInfo fileInfo = listOfFiles.at(i);
            QString fname = fileInfo.fileName().split("/").last();
            QString number = fname.split("_").last();
            number.replace(".root","");

            // if there is an extension, the file will be vts_data_VMMXXXXX_NNNN.root
            // with NNNN a number
            if(listOfFiles.size()==1 && number.contains("VMM"))
            {
                // no extension added already, this is the first time we're replicating
                max_ext_found = 0;
            }
            else if(listOfFiles.size()>1)
            {
                int ext = number.toInt(&ok,10);
                if(ext > max_ext_found) max_ext_found = ext;
            }
            else
            {
                throw std::runtime_error("Unknown filename format encountered");
            }
        } // i
    }

    return (max_ext_found+1);
}

bool FileManager::test_dir_exists(string test_name)
{
    TIter next(m_rfile->GetListOfKeys());
    TKey* key;
    while((key = (TKey*)next()))
    {
        TClass* c = gROOT->GetClass(key->GetClassName());
        if(!(c->InheritsFrom("TDirectory") || c->InheritsFrom("TDirectoryFile"))) continue;
        TDirectory* dir = (TDirectory*)key->ReadObj();
        if(dir->GetName() == test_name.c_str())
        {
            return true;
        }
    }
    return false;
}

bool FileManager::setup_output(vector<string> test_names)
{
    m_test_dir_map.clear();
    m_hist_dir_map.clear();
    m_tree_dir_map.clear();

    if(m_rfile)
        m_rfile->cd();
    else
    {
        return false;
    }

    for(const auto & test_name : test_names)
    {
        m_rfile->cd();
        stringstream test_dir_name;
        test_dir_name << test_name;
        if(m_rfile->GetDirectory(test_dir_name.str().c_str()) == 0x0)
        {
            auto top_dir = m_rfile->mkdir(test_dir_name.str().c_str());
            if(!top_dir)
            {
                log->critical("{0} - 0 Failed to create directory \"{1}\"",__VTFUNC__,test_dir_name.str());
                return false;
            }
            m_test_dir_map[test_name] = top_dir;
        }
        vector<string> hn = { "histograms", "trees" };
        for(auto & h : hn)
        {
            stringstream hdirname;
            hdirname << test_dir_name.str() << "/" << h;
            if(m_rfile->GetDirectory(hdirname.str().c_str()) == 0x0)
            {
                m_test_dir_map[test_name]->cd();
                auto hdir = m_rfile->mkdir(hdirname.str().c_str());
                if(!hdir)
                {
                    log->critical("{0} - 1 Failed to create directory \"{1}\"",__VTFUNC__, hdirname.str());
                    return false;
                }
                if(h=="histograms")
                {
                    m_hist_dir_map[test_name] = hdir;
                }
                else if(h=="trees")
                {
                    m_tree_dir_map[test_name] = hdir;
                }
            }
        }
    } // test_name
    return true;
}


bool FileManager::add_test_dir(string test_name)
{
    set_current_test(test_name);
    return true;

    /// TESTING [ENDS]
    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
    if(test_dir_exists(test_name))
    {
        log->info("{0} - TDirectory for test \"{1}\" already exists in output file",__VTFUNC__,test_name);
        return false;
    }
    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
    if(m_rfile)
        m_rfile->cd();

    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
    m_rfile->cd();
    m_current_test_dir = m_rfile->mkdir(test_name.c_str());
    if(m_current_test_dir)
    {
       // m_current_test_dir->Write();
        ///m_rfile->cd();
        //m_current_test_dir = m_rfile->mkdir(test_name.c_str());
        string name = m_current_test_dir->GetName();
        
        log->info("{0} - CURRENT DIR EXISTS {1} vs {2}",__VTFUNC__, name, test_name);
        if(name == test_name)
        {
            vector<string> dirnames = { "histograms", "trees" };
            for(const auto & n : dirnames)
            {
                stringstream d;
                d << "/" << test_name << "/" << n;
                m_rfile->cd();
                auto test_dir = m_rfile->GetDirectory(d.str().c_str());
                if(test_dir == 0x0)
                {
                    log->critical("{0} - {1} directory not found", __VTFUNC__, d.str());
                    //m_current_test_dir->cd();
                    m_current_hist_dir = m_rfile->mkdir(d.str().c_str());
                }
                else
                {
                    log->info("{0} - {1} directory found!",__VTFUNC__,d.str());
                }
            }
            //stringstream histname;
            //histname << "/" << test_name << "/histograms";
            //m_rfile->cd();
            //auto test_dir = m_rfile->GetDirectory(histname.str().c_str());
            //if(test_dir == 0x0)
            //{
            //    log->critical("{0} - {1} directory not found", __VTFUNC__, histname.str());
            //    //m_current_test_dir->cd();
            //    m_current_hist_dir = m_rfile->mkdir(histname.str().c_str());
            //}
            //else
            //{
            //}
        }
    }
   // if(m_current_hist_dir)
   // {
   // log->critical("{0} - {1}",__VTFUNC__,__LINE__);
   //     m_current_hist_dir->Write();
   // log->critical("{0} - {1}",__VTFUNC__,__LINE__);
   //     m_current_hist_dir->Close();
   //     //delete m_current_hist_dir;
   // log->critical("{0} - {1}",__VTFUNC__,__LINE__);
   // }
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//    if(m_current_tree_dir)
//    {
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//        m_current_tree_dir->Write();
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//        delete m_current_tree_dir;
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//    }
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//
//    if(m_current_test_dir)
//    {
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//        m_current_test_dir->Write();
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//        //m_current_test_dir->Close();
//        //delete m_current_test_dir;
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//    }
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//    set_current_test(test_name);
//    //m_current_test_dir = m_rfile->mkdir(test_name.c_str());
//    m_current_test_dir->cd();
//    log->critical("{0} - {1}",__VTFUNC__,__LINE__);
//
//    m_current_hist_dir = m_current_test_dir->mkdir("histograms");
//    m_current_tree_dir = m_current_test_dir->mkdir("trees");

    //if(m_current_tree_dir)
    //{
    //    m_current_tree_dir->Write();
    //    //m_current_tree_dir->Close();
    //    delete m_current_tree_dir;
    //}

    return true;
}

TDirectory* FileManager::get_test_dir(string dirname)
{
    TIter next(m_rfile->GetListOfKeys());
    TKey* key;
    while((key = (TKey*)next()))
    {
        TClass* c = gROOT->GetClass(key->GetClassName());
        if(!(c->InheritsFrom("TDirectory") || c->InheritsFrom("TDirectoryFile"))) continue;
        TDirectory* dir = (TDirectory*)key->ReadObj();
        stringstream objname;
        objname << dir->GetName();
        bool names_equal = (objname.str() == dirname);
        if(names_equal)
        {
            return dir;
        }
    }
    return nullptr;
}

TDirectory* FileManager::dir_has_dir(TDirectory* dir, string check)
{
    if(dir == nullptr || !dir)
    {
        log->warn("{0} - Provided an invalid TDirectory to check",__VTFUNC__);
        return nullptr;
    }

    log->warn("{0} - dir name = {1}",__VTFUNC__,dir->GetName());
    TIter next(dir->GetListOfKeys());
    TKey* key;
    while((key = (TKey*)next()))
    {
        TClass* c = gROOT->GetClass(key->GetClassName());
        log->error("{0} - FUCK {1}",__VTFUNC__,key->ReadObj()->GetName());
        if(!(c->InheritsFrom("TDirectory") || c->InheritsFrom("TDirectoryFile"))) continue;
        TDirectory* obj = (TDirectory*)key->ReadObj();
        stringstream objname;
        log->error("{0} - Obj name {1} in {2}",__VTFUNC__,obj->GetName(),check);
        objname << obj->GetName();
        bool names_equal = (objname.str() == check);
        if(names_equal)
        {
            return obj;
        }
    }
    return nullptr;
}

bool FileManager::store(TObject* obj)
{
    if(!obj)
    {
        log->warn("{0} - Attempting to store a NULL object",__VTFUNC__);
        return false;
    }
    if(!m_rfile)
    {
        log->error("{0} - Cannot store output object \"{1}\", output file is null",__VTFUNC__, obj->GetName());
        return true;
    }

    m_rfile->cd();
    stringstream top_dir;
    top_dir << current_test();

    bool ok = m_rfile->cd(top_dir.str().c_str());

    stringstream h_dir;
    TDirectory* store_dir = nullptr;
    if(obj->InheritsFrom("TTree") || obj->InheritsFrom("TChain"))
    {
        h_dir.str("");
        h_dir << "/" << current_test() << "/" << "trees" << "/";
        ok = m_rfile->cd(h_dir.str().c_str());
        if(!ok)
        {
            log->error("{0} - Unable to move to storage dir (={1}), output file not setup properly!",__VTFUNC__,h_dir.str());
            return false;
        }
    }
    else if(obj->InheritsFrom("TH1") ||
                obj->InheritsFrom("TH1F") ||
                obj->InheritsFrom("TH2") ||
                obj->InheritsFrom("TH2F") ||
                obj->InheritsFrom("TGraph") ||
                obj->InheritsFrom("TGraphErrors"))
    {
        h_dir.str("");
        h_dir << "/" << current_test() << "/" << "histograms" << "/";
        ok = m_rfile->cd(h_dir.str().c_str());
        if(!ok)
        {
            log->error("{0} - Unable to move to storage dir (={1}), output file not setup properly!",__VTFUNC__, h_dir.str());
            return false;
        }
        store_dir = m_rfile->GetDirectory(h_dir.str().c_str());
        if(obj->InheritsFrom("TH1") || obj->InheritsFrom("TH1F"))
        {
            ((TH1*)obj)->SetDirectory(store_dir);
        }
        else if(obj->InheritsFrom("TH2") || obj->InheritsFrom("TH2F"))
        {
            ((TH2*)obj)->SetDirectory(store_dir);
        }
    }
    if(store_dir == 0x0)
    {
        log->error("{0} - Storage directory (={1}) does not exist!",__VTFUNC__,h_dir.str());
        return false;
    }
    store_dir->cd();
    obj->Write();
    m_rfile->cd();
    return true;
}

} // namespace vts
