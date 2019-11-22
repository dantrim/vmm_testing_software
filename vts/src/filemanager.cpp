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
    log->info("{0}",__VTFUNC__);

    log->info("{0} - VMM serial {1}, Received output_config {2}",__VTFUNC__,vmm_serial_id, output_config.dump());


    m_vmm_serial_id = vmm_serial_id;
    m_output_config = output_config;
    m_current_test_dir = nullptr;
    m_current_hist_dir = nullptr;
    m_current_tree_dir = nullptr;

    ////// testing
    //TFile* rfile = new TFile("test.root", "recreate");

    //TDirectory* top_dir = rfile->mkdir("MyTest");
    //top_dir->cd();

    //TDirectory* tree_dir = top_dir->mkdir("trees");
    //TDirectory* plot_dir = top_dir->mkdir("plots");

    //rfile->Write();
    //delete rfile;
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
            return false;
        }
    }

    stringstream filename_no_ext;
    filename_no_ext << "vts_data_VMM";
    filename_no_ext << m_vmm_serial_id;
    int ext_no = existing_files(outdir.str(), filename_no_ext.str());
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
    }
    else
    {
        m_rfile = new TFile(testfile.str().c_str(), "UPDATE");
    }

    return true;
}

int FileManager::existing_files(string dirname, string filename_no_ext)
{
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

bool FileManager::add_test_dir(string test_name)
{
    if(test_dir_exists(test_name))
    {
        log->info("{0} - TDirectory for test \"{1}\" already exists in output file",__VTFUNC__,test_name);
        return false;
    }
    if(m_current_test_dir)
    {
        delete m_current_test_dir;
    }
    set_current_test(test_name);
    m_current_test_dir = m_rfile->mkdir(test_name.c_str());

    if(m_current_hist_dir)
    {
        delete m_current_hist_dir;
    }

    if(m_current_tree_dir)
    {
        delete m_current_tree_dir;
    }

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
    //TDirectory* current_test_dir = get_test_dir(current_test());
    if(!m_current_test_dir)
        return false;
    m_current_test_dir->cd();

    TDirectory* store_dir = nullptr;
    std::string check_dir_name = "";
    if(obj->InheritsFrom("TTree") || obj->InheritsFrom("TChain"))
    {
        if(!m_current_tree_dir)
            m_current_tree_dir = m_current_test_dir->mkdir("trees");
        store_dir = m_current_tree_dir;
        ((TTree*)obj)->SetDirectory(m_current_tree_dir);
        check_dir_name = "trees";
    }
    else if(obj->InheritsFrom("TH1") ||
                obj->InheritsFrom("TH1F") ||
                obj->InheritsFrom("TH2") ||
                obj->InheritsFrom("TH2F") ||
                obj->InheritsFrom("TGraph") ||
                obj->InheritsFrom("TGraphErrors"))
    {
        if(!m_current_hist_dir)
            m_current_hist_dir = m_current_test_dir->mkdir("histograms");
        store_dir = m_current_hist_dir;
        if(obj->InheritsFrom("TH1") || obj->InheritsFrom("TH1F"))
        {
            ((TH1*)obj)->SetDirectory(m_current_hist_dir);
        }
        else if(obj->InheritsFrom("TH2") || obj->InheritsFrom("TH2F"))
        {
            ((TH2*)obj)->SetDirectory(m_current_hist_dir);
        }
        //else if(obj->InheritsFrom("TGraph"))
        //{
        //    ((TGraph*)obj)->SetDirectory(m_current_hist_dir);
        //}
        check_dir_name = "histograms";
    }

    if(check_dir_name == "")
    {
        log->error("{0} - Unable to store ROOT object of provided type (object name: {1}",__VTFUNC__,obj->GetName());
        return false;
    }

    store_dir->cd();
    obj->Write();
    m_rfile->cd();
    return true;

//    TDirectory* store_dir = nullptr;
//    if(!dir_has_dir(current_test_dir,check_dir_name))
//    {
//        log->info("{0} - BALLS Creating dir name {1}/{2}",__VTFUNC__,current_test_dir->GetName(), check_dir_name);
//        store_dir = current_test_dir->mkdir(check_dir_name.c_str());
//    }
//    store_dir = dir_has_dir(current_test_dir, check_dir_name);
//    cout << "store_dir 1 = " << store_dir << " " << __LINE__ << endl;
//    int attempts = 0;
//    while(store_dir==nullptr)
//    {
//        if(store_dir != nullptr) break;
//        if(attempts>5)
//        {
//            log->error("{0} - Unable to create {1}/{2} TDirectory!",__VTFUNC__,current_test(),check_dir_name);
//            return false;
//        }
//        log->warn("{0} - X Creating {1}/{2} directory",__VTFUNC__,current_test(),check_dir_name);
//        store_dir =  current_test_dir->mkdir(check_dir_name.c_str());
//        //store_dir = dir_has_dir(current_test_dir, check_dir_name);
//        cout << "store_dir 2 = " << store_dir << " " << __LINE__ << endl;
//        attempts++;
//    }
//    m_directories.push_back(store_dir);
//    
//    store_dir->cd();
//    obj->Write();
//    m_rfile->cd();
//
//    return true;
}

} // namespace vts
