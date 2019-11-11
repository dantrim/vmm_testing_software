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
    json global_registers_json = vmm_spi.at("global_registers");
//    json channel_registers_json = vmm_spi.at("channel_registers");

    vector<string> global_registers = 
            spi_global_register_vec(global_registers_json);
//    vector<string> channel_registers =
//            spi_channel_register_vec(channel_registers_json);

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
    spi1_0.replace(pos,1,QString::number(pos_neg(jreg.at("sp"))));//config().globalSettings().sp));
    pos++;

    // sdp
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sdp"))));//config().globalSettings().sdp));
    pos++;

    // sbmx
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbmx"))));//config().globalSettings().sbmx));
    pos++;

    // sbft
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbft"))));//config().globalSettings().sbft));
    pos++;

    // sbfp
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbfp"))));//config().globalSettings().sbfp));
    pos++;

    // sbfm
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sbfm"))));//config().globalSettings().sbfm));
    pos++;

    // slg
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("slg"), true)));//config().globalSettings().slg));
    pos++;

    // sm5-sm0
    tmp = QString("%1").arg(std::stoi(jreg.at("sm5").get<std::string>()),6,2,QChar('0'));//config().globalSettings().sm5,6,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // scmx
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("scmx"))));//config().globalSettings().scmx));
    pos++;

    // sfa
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sfa"))));//config().globalSettings().sfa));
    pos++;

    // sfam
    std::map<std::string, int> sfam_map = {{"PEAK",1},{"THRESHOLD",0}};
    spi1_0.replace(pos,1,QString::number(sfam_map.at(jreg.at("sfam"))));//config().globalSettings().sfam));
    pos++;

    // st
    std::map<std::string, int> st_map = {{"200",0},{"100",1},{"50",2},{"25",3}};
    tmp = QString("%1").arg(st_map.at(jreg.at("st")),2,2,QChar('0'));//config().globalSettings().st,2,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sfm
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sfm"))));//config().globalSettings().sfm));
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
                
    tmp = QString("%1").arg(sg_map.at(jreg.at("sg")),3,2,QChar('0'));//config().globalSettings().sg,3,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sng
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sng"))));//config().globalSettings().sng));
    pos++;

    std::map<std::string, int> stpp_map = {{"TtP",0},{"ToT",0},{"PtP",1},{"PtT",1}};
    std::map<std::string, int> stot_map = {{"TtP",0},{"ToT",1},{"PtP",0},{"PtT",1}};

    // stot
    spi1_0.replace(pos,1,QString::number(stot_map.at(jreg.at("direct_timing_mode"))));//config().globalSettings().stot));
    pos++;

    // sttt
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("sttt"))));//config().globalSettings().sttt));
    pos++;

    // ssh
    spi1_0.replace(pos,1,QString::number(ena_dis(jreg.at("ssh"))));//config().globalSettings().ssh));
    pos++;

    // stc
    std::map<std::string, int> stc_map = {{"60",0},{"100",1},{"350",2},{"650",3}};
    tmp = QString("%1").arg(stc_map.at(jreg.at("stc")),2,2,QChar('0'));//config().globalSettings().stc,2,2,QChar('0'));
    spi1_0.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // first 4 bits of sdt
    uint32_t sdt_tmp = 0;
    sdt_tmp |= ( (0x3C0 & std::stoi(jreg.at("sdt_dac").get<std::string>())) >> 6 );//config().globalSettings().sdt_dac) >> 6 );
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
    sdt_tmp |= ((std::stoi(jreg.at("sdt_dac").get<std::string>()) & 0x3F));//config().globalSettings().sdt_dac & 0x3F));// & 0x3F;
    //sdt_tmp |= ((config().globalSettings().sdt_dac & 0x3F0) >> 4);// & 0x3F;
    tmp = QString("%1").arg(sdt_tmp,6,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sdp
    tmp = QString("%1").arg(std::stoi(jreg.at("sdp_dac").get<std::string>()),10,2,QChar('0'));//config().globalSettings().sdp_dac,10,2,QChar('0'));
    //tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc010b:sc110b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc10b").get<std::string>()),2,2,QChar('0'));//config().globalSettings().sc10b,2,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc08b:sc18b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc08b").get<std::string>()),2,2,QChar('0'));//config().globalSettings().sc8b,2,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // sc06b:sc26b
    tmp = QString("%1").arg(std::stoi(jreg.at("sc06b").get<std::string>()),3,2,QChar('0'));//config().globalSettings().sc6b,3,2,QChar('0'));
    tmp = reverseString(tmp);
    spi1_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // s8b
    spi1_1.replace(pos,1,QString::number(std::stoi(jreg.at("s8b").get<std::string>())));//config().globalSettings().s8b));
    pos++;

    // s6b
    spi1_1.replace(pos,1,QString::number(std::stoi(jreg.at("s6b").get<std::string>())));//config().globalSettings().s6b));
    pos++;

    // s10b
    spi1_1.replace(pos,1,QString::number(std::stoi(jreg.at("s10b").get<std::string>())));//config().globalSettings().s10b));
    pos++;

    // sdcks
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdcks"))));//config().globalSettings().sdcks));
    pos++;

    // sdcka
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdcka"))));//config().globalSettings().sdcka));
    pos++;

    // sdck6b
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdck6b"))));//config().globalSettings().sdck6b));
    pos++;

    // sdrv
    spi1_1.replace(pos,1,QString::number(ena_dis(jreg.at("sdrv"))));//config().globalSettings().sdrv));
    pos++;

    // stpp
    spi1_1.replace(pos,1,QString::number(stpp_map.at(jreg.at("direct_timing_mode"))));//config().globalSettings().stpp));
    pos++;

    // res00

    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-2 [res0:reset]
    QString spi1_2 = bit32_empty;
    pos = 0;

    pos += 4; // first 4 reserved

    // slvs
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvs"))));//config().globalSettings().slvs));
    pos++;

    // s32
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("s32"))));//config().globalSettings().s32));
    pos++;

    // stcr
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("stcr"))));//config().globalSettings().stcr));
    pos++;

    // ssart
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("ssart"))));//config().globalSettings().ssart));
    pos++;

    // srec
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("srec"))));//config().globalSettings().srec));
    pos++;

    // stlc
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("stlc"))));//config().globalSettings().stlc));
    pos++;

    // sbip
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sbip"))));//config().globalSettings().sbip));
    pos++;

    // srat
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("srat"))));//config().globalSettings().srat));
    pos++;

    // sfrst
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sfrst"))));//config().globalSettings().sfrst));
    pos++;

    // slvsbc
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsbc"))));//config().globalSettings().slvsbc));
    pos++;

    // slvstp
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstp"))));//config().globalSettings().slvstp));
    pos++;

    // slvstk
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstk"))));//config().globalSettings().slvstk));
    pos++;

    // slvsdt
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsdt"))));//config().globalSettings().slvsdt));
    pos++;

    // slvsart
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsart"))));//config().globalSettings().slvsart));
    pos++;

    // slvstki
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvstki"))));//config().globalSettings().slvstki));
    pos++;

    // slvsena
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvsena"))));//config().globalSettings().slvsena));
    pos++;

    // slvs6b
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("slvs6b"))));//config().globalSettings().slvs6b));
    pos++;

    // sL0enaV
    spi1_2.replace(pos,1,QString::number(ena_dis(jreg.at("sL0enaV"))));//config().globalSettings().sL0enaV));
    pos++; // we have 8 nu before the reset bits

    //slh
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("slh").get<std::string>())));//config().globalSettings().slh));
    pos++;

    //slxh
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("slxh").get<std::string>())));//config().globalSettings().slxh));
    pos++;

    //stgc
    spi1_2.replace(pos,1,QString::number(std::stoi(jreg.at("stgc").get<std::string>())));//config().globalSettings().stgc));
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
    spi2_0.replace(pos,1,QString::number(ena_dis(jreg.at("nskipm"))));//config().globalSettings().nskipm));


    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-1 [sL0cktest:rollover]

    QString spi2_1 = bit32_empty;
    pos = 0;

    // sL0cktest
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0cktest"))));//config().globalSettings().sL0cktest));
    pos++;

    // sL0dckinv
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0dckinv"))));//config().globalSettings().sL0dckinv));
    pos++;

    // sL0ckinv
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0ckinv"))));//config().globalSettings().sL0ckinv));
    pos++;

    // sL0ena
    spi2_1.replace(pos,1,QString::number(ena_dis(jreg.at("sL0ena"))));//config().globalSettings().sL0ena));
    pos++;

    // truncate
    tmp = QString("%1").arg(std::stoi(jreg.at("truncate").get<std::string>()),6,2,QChar('0'));//config().globalSettings().truncate,6,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // nskip
    tmp = QString("%1").arg(std::stoi(jreg.at("nskip").get<std::string>()),7,2,QChar('0'));//config().globalSettings().nskip,7,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // window
    tmp = QString("%1").arg(std::stoi(jreg.at("window").get<std::string>()),3,2,QChar('0'));//config().globalSettings().window,3,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // rollover
    tmp = QString("%1").arg(std::stoi(jreg.at("rollover").get<std::string>()),12,2,QChar('0'));//config().globalSettings().rollover,12,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_1.replace(pos,tmp.size(),tmp);


    ////////////////////////////////////////////////////////////////////
    ////////////////// 32-2 [l0offset:offset]

    QString spi2_2 = bit32_empty;
    pos = 0;

    // l0offset
    tmp = QString("%1").arg(std::stoi(jreg.at("l0offset").get<std::string>()),12,2,QChar('0'));//config().globalSettings().l0offset,12,2,QChar('0'));
    tmp = reverseString(tmp);
    spi2_2.replace(pos,tmp.size(),tmp);
    pos+=tmp.size();

    // offset
    tmp = QString("%1").arg(std::stoi(jreg.at("offset").get<std::string>()),12,2,QChar('0'));//config().globalSettings().offset,12,2,QChar('0'));
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
    vector<string> out;
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
