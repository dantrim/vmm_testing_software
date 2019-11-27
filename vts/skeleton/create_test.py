#!/usr/bin/env python3

import sys, os
import argparse
from pathlib import Path
import json

def is_good_dir(path) :
    return path.exists() and path.is_dir()

def is_good_file(path) :
    return path.exists() and path.is_file()

def get_top_level_dir() :

    cwd = Path.cwd()
    for p in cwd.parents :
        if p.parts[-1] == "vmm_testing_software" :
            break
    if not is_good_dir(p) :
        return None
    return p

def get_code_dirs() :

    p = get_top_level_dir()
    p_test_include = p / "vts/include/tests/"
    if not is_good_dir(p_test_include) :
        p_test_include = None
    p_test_src = p / "vts/src/tests/"
    if not is_good_dir(p_test_src) :
        p_test_src = None
    return p_test_include, p_test_src

def test_src_dir() :
    return get_code_dirs()[1]
def test_include_dir() :
    return get_code_dirs()[0]

def skeleton_code_dirs() :

    p = get_top_level_dir()
    p_skel_include = p / "vts/skeleton/include/"
    if not is_good_dir(p_skel_include) :
        p_skel_include = None
    p_skel_src = p / "vts/skeleton/src/"
    if not is_good_dir(p_skel_src) :
        p_skel_src = None
    return p_skel_include, p_skel_src

def skeleton_src_dir() :
    return skeleton_code_dirs()[1]
def skeleton_include_dir() :
    return skeleton_code_dirs()[0]

def create_new_source_file(test_name = "", outname = "", skeleton = "") :

    with open(outname, "w") as ofile :
        with open(skeleton, "r") as ifile :
            for line in ifile :
                line = line.replace("SKELETONTEST", test_name)
                ofile.write("{}".format(line))
    return True

def make_test_source_files(args) :

    which_test = { False : "VTSTestSKELETONTESTVMM.cpp",
                    True : "VTSTestSKELETONTESTXADC.cpp" }[args.xadc]
    p_skel_test_src = skeleton_src_dir() / which_test
    if not is_good_file(p_skel_test_src) :
        return False
    p_skel_test_include = skeleton_include_dir() / which_test.replace("VMM","").replace("XADC","").replace(".cpp",".h")
    if not is_good_file(p_skel_test_include) :
        return False

    out_src_name = str(which_test).replace("VMM","").replace("XADC","").replace("SKELETONTEST",args.test)
    out_include_name = str(which_test).replace("VMM","").replace("XADC","").replace("SKELETONTEST",args.test).replace(".cpp",".h")

    create_new_source_file(test_name = args.test, outname = out_src_name, skeleton = str(p_skel_test_src))
    p = Path(out_src_name)
    if not is_good_file(p) :
        print("Failed to create {}".format(out_src_name))
        return False

    create_new_source_file(test_name = args.test, outname = out_include_name, skeleton = str(p_skel_test_include))
    p = Path(out_include_name)
    if not is_good_file(p) :
        print("Failed to create {}".format(out_include_name))
        return False

    return True

def update_test_type_source(source_code_dir, test_name) :

    p = source_code_dir / "vts_test_types.cpp"
    if not is_good_file(p) :
        return False
    outname = "vts_test_types_UPDATED.cpp"
    with open(outname, "w") as ofile :
        new_lines = []
        with open(str(p), "r") as ifile :
            lines = ifile.readlines()
            current_function = ""
            old_test_name = ""
            for iline, line in enumerate(lines) :
                if "ToStr" in line :
                    current_function = "ToStr"
                elif "StrTo" in line :
                    current_function = "StrTo"
                if "VTSTESTTYPEINVALID" in line and "VTSTestType::" in line and current_function == "ToStr" :
                    new_line = lines[iline-1]
                    old_test_name = new_line.split("::")[1].split(":")[0].strip()
                    new_line = new_line.replace(old_test_name, test_name)
                    new_lines.append(new_line)
                if "VTSTestType::VTSTESTTYPEINVALID" in line and "VTSTestType::" in line and current_function == "StrTo" :
                    new_line = lines[iline-1]
                    new_line = new_line.replace(old_test_name, test_name)
                    new_lines.append(new_line)
                new_lines.append(line)
        for iline, line in enumerate(new_lines) :
            if line.strip() == "{" and new_lines[iline-1].strip() == "{" : continue
            if line.strip() == "}" and new_lines[iline-1].strip() == "}" : continue
            ofile.write(line)
    return True, old_test_name

def update_test_type_enum(include_code_dir, test_name, old_test_name) :

    p = include_code_dir / "vts_test_types.h"
    if not is_good_file(p) :
        return False
    outname = "vts_test_types_UPDATED.h"
    with open(outname, "w") as ofile :
        new_lines = []
        with open(str(p), "r") as ifile :
            lines = ifile.readlines()
            in_enum = False
            for iline, line in enumerate(lines) :
                if "enum VTSTestType" in line :
                    in_enum = True
                if in_enum and line.strip() == "};" :
                    in_enum = False
                if in_enum and ",{}".format(old_test_name) in line :
                    new_lines.append(line)
                    new_line = line.replace(old_test_name, test_name)
                    new_lines.append(new_line)
                else :
                    new_lines.append(line)
        for line in new_lines :
            ofile.write(line)
    return True

def update_tests_include(test_name, old_test_name) :

    pinclude = test_include_dir()
    p = pinclude / "tests.h"
    if not is_good_file(p) :
        return False
    outname = "tests_UPDATED.h"
    with open(outname, "w") as ofile :
        new_lines = []
        with open(str(p), "r") as ifile :
            lines = ifile.readlines()
            for iline, line in enumerate(lines) :
                if "VTSTest{}.h".format(old_test_name) in line :
                    new_lines.append(line)
                    new_line = "#include \"tests/VTSTest{}.h\"".format(test_name)
                    new_lines.append("{}\n".format(new_line))
                else :
                    new_lines.append(line)
        for line in new_lines :
            ofile.write(line)
    return True

def update_test_init(test_name) :

    p = skeleton_src_dir() / "snippet_vts_test.cpp"
    if not is_good_file(p) :
        return False

    outname = "snippet_vts_test_UPDATED.cpp"
    with open(outname, "w") as ofile :
        with open(str(p), "r") as ifile :
            for line in ifile :
                line = line.replace("SKELETONTEST",test_name)
                ofile.write(line)
    return True

def update_test_types(args) :

    source_code_dir = Path(str(test_src_dir()).replace("tests",""))
    include_code_dir = Path(str(test_include_dir()).replace("tests",""))

    status, old_test_name = update_test_type_source(source_code_dir, args.test)
    if not status :
        return False
    if not update_test_type_enum(include_code_dir, args.test, old_test_name) :
        return False

    if not update_tests_include(args.test, old_test_name) :
        return False

    if not update_test_init(args.test) :
        return False

    return True

def create_test_config(args) :

    config = {}
    config["test_type"] = str(args.test)
    test_data = {}

    pvmm = get_top_level_dir()
    pconfig_frontend = pvmm / "vts/config/frontend"
    if not is_good_dir(pconfig_frontend) :
        return False

    config_files = pconfig_frontend.glob("*default.json")
    fpga_config = ""
    vmm_config = ""
    for cfg in config_files :
        if "vmm_spi" in str(cfg) :
            test_data["base_config_global"] = str(cfg)
        elif "fpga_register" in str(cfg) :
            test_data["base_config_fpga"] = str(cfg)
    config["test_data"] = test_data

    outname = "test_config_{}.json".format(args.test)
    with open(outname, "w") as ofile :
        json.dump(config, ofile, indent = 4)
    return True

def main() :

    parser = argparse.ArgumentParser(description = "Create the needed code for a new test!")
    parser.add_argument("-t","--test", default = "",
        help = "The name to be given to the test",
        required = True
    )
    parser.add_argument("--xadc", action = "store_true",
        default = False,
        help = "Uses xADC decoding (default is VMM)"
    )
    args = parser.parse_args()

    if not make_test_source_files(args) :
        print("Failed to create test")
        sys.exit(1)

    if not update_test_types(args) :
        print("Failed to create updated test types enum")
        sys.exit(1)

    if not create_test_config(args) :
        print("Failed to make test config file")
        sys.exit(1)

if __name__ == "__main__" :
    main()
