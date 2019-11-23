#!/usr/bin/env python3

import sys
import json
from pathlib import Path
from collections import OrderedDict

def vmm_testing_software_path() :

    cwd = Path.cwd()
    for p in cwd.parents :
        if p.parts[-1] == "vmm_testing_software" :
            break
    if p.exists() :
        return p
    else :
        return None

def vts_config_path() :

    p_vmm_testing_software = vmm_testing_software_path()
    if not p_vmm_testing_software :
        return None

    p_config = p_vmm_testing_software / "vts" / "config"
    if p_config.exists() :
        return p_config
    else :
        return None

def default_vts_config() :
    """
    Inspect the current directory and get the default config file
    """

    p_config_dir = vts_config_path()
    if not p_config_dir :
        return None

    p_config_file = p_config_dir / "vts_default.json"
    if p_config_file.exists() :
        return p_config_file
    else :
        return None

def get_defined_tests() :


    p_config_dir = vts_config_path()
    if not p_config_dir :
        return {}
    print("Looking for defined tests")

    p_test_dir = p_config_dir / "tests"
    if not p_test_dir.exists() :
        return {}
    test_config_files = list(p_test_dir.glob("test_config*.json"))
    print("Found {} test config files".format(len(test_config_files)))

    test_dict = {}
    for ptest in test_config_files :
        with open(ptest, "r") as testfile :
            testdata = json.load(testfile)
        test_type = testdata["test_type"]
        test_dict[test_type] = ptest

    test_priority_list = {}
    test_priority_file = p_test_dir / "test_priority.json"
    if test_priority_file.exists() :
        with open(test_priority_file, "r") as tpfile :
            test_priority_dict = json.load(tpfile)
        test_priority_list = test_priority_dict["test_priority"]
        tmp_test_dict = OrderedDict()
        for test_name in test_priority_list :
            for tn, tf in test_dict.items() :
                if tn == test_name :
                    tmp_test_dict[test_name] = tf
                    break
        test_dict = tmp_test_dict

    return p_test_dir, test_dict
