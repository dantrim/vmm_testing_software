//vts
#include "spi_builder.h"

//std/stl
#include <iostream>
using namespace std;

namespace vts
{
namespace spi
{

vector<string> spi_string_vec(json vmm_spi)
{
    json vmm_spi_json = vmm_spi.at("vmm_spi");
    json global_registers_json = vmm_spi_json.at("global_registers");
    json channel_registers_json = vmm_spi_json.at("channel_registers");

    vector<string> global_registers = 
            spi_global_register_vec(global_registers_json);
    vector<string> channel_registers =
            spi_channel_register_vec(channel_registers_json);

//    cout << "============== GLOBAL REGISTERS ===============" << endl;
//    for(size_t i = 0; i < global_registers.size(); i++)
//    {
//        cout << "["<<i<<"] " << global_registers.at(i) << endl;
//    }
//    cout << "============== CHANNEL REGISTERS ===============" << endl;
//    for(size_t i = 0; i < channel_registers.size(); i++)
//    {
//        cout << "["<<i<<"] " << channel_registers.at(i) << endl;
//    }

    return global_registers;
}

vector<string> spi_global_register_vec(json jreg, bool reset)
{
    QString bit32_empty = "00000000000000000000000000000000";
    QString tmp;

    //////////////////////////////////////////////////////////////////
    // Global Bank 1
    //////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-1 [sp:sdt3]
    QString spi1_0 = bit32_empty;
    int pos = 0;

    // sp
    spi1_0.replace(pos,1,QString::number(pos_neg(jreg.at("sp"))));
    pos++;

    // sdp
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sdp"))));
    pos++;

    // sbmx
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbmx"))));
    pos++;

    // sbft
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbft"))));
    pos++;

    // sbfp
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbfp"))));
    pos++;

    // sbfm
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbfm"))));
    pos++;

    // slg
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("slg"), true)));
    pos++;

    // sm5-sm0
    tmp = QString("%1").arg(std::stoi(jreg.at("sm5").get<std::string>()),6,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // scmx
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("scmx"))));
    pos++;

    // sfa
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sfa"))));
    pos++;

    // sfam
    std::map<std::string, int> sfam_map = {{"PEAK",1},{"THRESHOLD",0}};
    spi1_0.replace(pos,1,QString::number(sfam_map.at(jreg.at("sfam"))));
    pos++;

    // st
    std::map<std::string, int> st_map = {{"200",0},{"100",1},{"50",2},{"25",3}};
    tmp = QString("%1").arg(st_map.at(jreg.at("st")),2,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sfm
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sfm"))));
    pos++;

    // sg
    std::map<std::string, int> sg_map = {{"0.5",0},
                                            {"1.0",1},
                                            {"1",1},
                                            {"3.0",2},
                                            {"3",2},
                                            {"4.5",3},
                                            {"6.0",4},
                                            {"6",4},
                                            {"9.0",5},
                                            {"9",5},
                                            {"12.0",6},
                                            {"12",6},
                                            {"16.0",7},
                                            {"16",7}
                                        };
                
    tmp = QString("%1").arg(sg_map.at(jreg.at("sg")),3,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sng
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sng"))));
    pos++;

    std::map<std::string, int> stpp_map = {{"TtP",0},{"ToT",0},{"PtP",1},{"PtT",1}};
    std::map<std::string, int> stot_map = {{"TtP",0},{"ToT",1},{"PtP",0},{"PtT",1}};

    // stot
    spi1_0.replace(pos,1,QString::number(stot_map.at(jreg.at("direct_timing_mode"))));
    pos++;

    // sttt
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sttt"))));
    pos++;

    // ssh
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("ssh"))));
    pos++;

    // stc
    std::map<std::string, int> stc_map = {{"60",0},{"100",1},{"350",2},{"650",3}};
    tmp = QString("%1").arg(stc_map.at(jreg.at("stc")),2,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // first 4 bits of sdt
    uint32_t sdt_tmp = 0;
    cout << "SDT_DAC = " << std::stoi(jreg.at("sdt_dac").get<std::string>()) << endl;
    sdt_tmp |= ( (0x3C0 & std::stoi(jreg.at("sdt_dac").get<std::string>())) >> 6 );
    //sdt_tmp |= ( 0xF & config().globalSettings().sdt_dac );
    tmp = QString("%1").arg(sdt_tmp,4,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-1 [sd4:res00]
    QString spi1_1 = bit32_empty;
    pos = 0;

    // last 6 bits of sdt
    sdt_tmp = 0;
    sdt_tmp |= ((std::stoi(jreg.at("sdt_dac").get<std::string>()) & 0x3F));
    //sdt_tmp |= ((config().globalSettings().sdt_dac & 0x3F0) >> 4);// & 0x3F;
    tmp = QString("%1").arg(sdt_tmp,6,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sdp
    tmp = QString("%1").arg(std::stoi(jreg.at("sdp_dac").get<std::string>()),10,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc010b:sc110b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc10b").get<std::string>()),2,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc08b:sc18b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc08b").get<std::string>()),2,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc06b:sc26b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc06b").get<std::string>()),3,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // s8b
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("s8b"))));
    pos++;

    // s6b
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("s6b"))));
    pos++;

    // s10b
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("s10b"))));
    pos++;

    // sdcks
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdcks"))));
    pos++;

    // sdcka
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdcka"))));
    pos++;

    // sdck6b
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdck6b"))));
    pos++;

    // sdrv
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdrv"))));
    pos++;

    // stpp
    spi1_1.replace(pos,1,QString::number(stpp_map.at(jreg.at("direct_timing_mode"))));
    pos++;

    // res00

    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-2 [res0:reset]
    QString spi1_2 = bit32_empty;
    pos = 0;

    pos += 4; // first 4 reserved

    // slvs
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvs"))));
    pos++;

    // s32
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("s32"))));
    pos++;

    // stcr
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("stcr"))));
    pos++;

    // ssart
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("ssart"))));
    pos++;

    // srec
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("srec"))));
    pos++;

    // stlc
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("stlc"))));
    pos++;

    // sbip
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sbip"))));
    pos++;

    // srat
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("srat"))));
    pos++;

    // sfrst
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sfrst"))));
    pos++;

    // slvsbc
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsbc"))));
    pos++;

    // slvstp
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstp"))));
    pos++;

    // slvstk
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstk"))));
    pos++;

    // slvsdt
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsdt"))));
    pos++;

    // slvsart
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsart"))));
    pos++;

    // slvstki
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstki"))));
    pos++;

    // slvsena
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsena"))));
    pos++;

    // slvs6b
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvs6b"))));
    pos++;

    // sL0enaV
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sL0enaV"))));
    pos++; // we have 8 nu before the reset bits

    //slh
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("slh").get<std::string>())));
    pos++;

    //slxh
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("slxh").get<std::string>())));
    pos++;

    //stgc
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("stgc").get<std::string>())));
    pos+=6;
    // reset
    uint32_t reset_flags = 0;
    if(reset)
    {
        // raise last 2 bits high if doing reset
        reset_flags = 3;
    }
    tmp = QString("%1").arg(reset_flags,2,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_2.replace(pos, tmp.size(),tmp);

    //////////////////////////////////////////////////////////////////
    // Global Bank 2
    //////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-0 [nu:nskipm]
    QString spi2_0 = bit32_empty;
    pos = 0;

    // bits [0:30] are not-used
    pos+=31;

    // nskipm
    spi2_0.replace(pos,1,QString::number(ena_dis(jreg.at("nskipm"))));


    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-1 [sL0cktest:rollover]

    QString spi2_1 = bit32_empty;
    pos = 0;

    // sL0cktest
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0cktest"))));
    pos++;

    // sL0dckinv
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0dckinv"))));
    pos++;

    // sL0ckinv
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0ckinv"))));
    pos++;

    // sL0ena
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0ena"))));
    pos++;

    // truncate
    tmp = QString("%1").arg(std::stoi(jreg.at("truncate").get<std::string>()),6,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // nskip
    tmp = QString("%1").arg(std::stoi(jreg.at("nskip").get<std::string>()),7,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // window
    tmp = QString("%1").arg(std::stoi(jreg.at("window").get<std::string>()),3,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // rollover
    tmp = QString("%1").arg(std::stoi(jreg.at("rollover").get<std::string>()),12,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);


    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-2 [l0offset:offset]

    QString spi2_2 = bit32_empty;
    pos = 0;

    // l0offset
    tmp = QString("%1").arg(std::stoi(jreg.at("l0offset").get<std::string>()),12,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_2.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // offset
    tmp = QString("%1").arg(std::stoi(jreg.at("offset").get<std::string>()),12,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_2.replace(pos,tmp.size(),tmp);

    // done
    vector<string> out;
    out.push_back(spi1_0.toStdString());
    out.push_back(spi1_1.toStdString());
    out.push_back(spi1_2.toStdString());
    out.push_back(spi2_0.toStdString());
    out.push_back(spi2_1.toStdString());
    out.push_back(spi2_2.toStdString());
    return out;
}

vector<string> spi_channel_register_vec(json jreg)
{
    int pos = 0;
    QString tmp;
    QString reg;
    QString bit32_empty = "00000000000000000000000000000000";

    

    bool ok;
    uint64_t sc  = QString::fromStdString(jreg.at("sc").get<std::string>()).toULong(&ok,0);
    uint64_t sl  = QString::fromStdString(jreg.at("sl").get<std::string>()).toULong(&ok,0);
    uint64_t sth = QString::fromStdString(jreg.at("sth").get<std::string>()).toULong(&ok,0);
    uint64_t st  = QString::fromStdString(jreg.at("st").get<std::string>()).toULong(&ok,0);
    uint64_t sm  = QString::fromStdString(jreg.at("sm").get<std::string>()).toULong(&ok,0);
    uint64_t smx = QString::fromStdString(jreg.at("smx").get<std::string>()).toULong(&ok,0);

    // hardcode for testing
    cout << "WARNING HARDCODING NON-BOOL CHANNEL BITS" << endl;
    vector<string> out;
    for(int i = 0; i < 64; i++)
    {
        reg = bit32_empty;
        pos = 0;

        int chan_sc = ((sc >> i) & 0x1) ? 1 : 0;
        int chan_sl = ((sl >> i) & 0x1) ? 1 : 0;
        int chan_sth = ((sth >> i) & 0x1) ? 1 : 0;
        int chan_st = ((st >> i) & 0x1) ? 1 : 0;
        int chan_sm = ((sm >> i) & 0x1) ? 1 : 0;
        int chan_smx = ((smx >> i) & 0x1) ? 1 : 0;

        reg.replace(pos,1,QString::number(chan_sc));
        pos++;

        reg.replace(pos,1,QString::number(chan_sl));
        pos++;

        reg.replace(pos,1,QString::number(chan_st));
        pos++;

        reg.replace(pos,1,QString::number(chan_sth));
        pos++;

        reg.replace(pos,1,QString::number(chan_sm));
        pos++;

        reg.replace(pos,1,QString::number(chan_smx));
        pos++;

        // sd (trim)
        tmp = QString("%1").arg(0,5,2,QChar('0'));
        tmp = reverseString(tmp);
        reg.replace(pos,tmp.size(),tmp);
        pos+=tmp.size();

        // sz10b 
        tmp = QString("%1").arg(0,5,2,QChar('0'));
        tmp = reverseString(tmp);
        reg.replace(pos,tmp.size(),tmp);
        pos+=tmp.size();

        // sz8b
        tmp = QString("%1").arg(0,4,2,QChar('0'));
        tmp = reverseString(tmp);
        reg.replace(pos,tmp.size(),tmp);
        pos+=tmp.size();

        // sz6b
        tmp = QString("%1").arg(0,3,2,QChar('0'));
        tmp = reverseString(tmp);
        reg.replace(pos,tmp.size(),tmp);
        pos+=tmp.size();

        // push back
        reg = reverseString(reg);
        out.push_back(reg.toStdString());
    } // i

    // done
    return out;
}

int ena_dis(std::string val, bool inverted)
{
    if(val == "ENABLED")
    {
        return inverted ? 0 : 1;
    }
    else if(val == "DISABLED")
    {
        return inverted ? 1 : 0;
    }
    else
    {
        cout << "SPIBUILDER ERR ena_dis" << endl;
        return inverted ? 1 : 0;
    }
}

int pos_neg(std::string val, bool inverted)
{
    if(val == "POSITIVE")
    {
        return inverted ? 0 : 1;
    }
    else if(val == "NEGATIVE")
    {
        return inverted ? 1 : 0;
    }
    else
    {
        cout << "SPIBUILDER ERR pos_neg" << endl;
        return inverted ? 1 : 0;
    }
}

QString reverseString(QString str)
{
    QString tmp = str;
    QByteArray ba = tmp.toLocal8Bit();
    char *d = ba.data();
    std::reverse(d,d+tmp.length());
    tmp = QString(d);
    return tmp;
}



} // namespace spi
} // namespace vts
