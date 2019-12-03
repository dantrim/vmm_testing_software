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

def default_tests_dir() :

    p_config_dir = vts_config_path()
    if not p_config_dir :
        return None
    p_test_dir = p_config_dir / "tests"
    exists_and_is_dir = p_test_dir.exists() and p_test_dir.is_dir()
    if not exists_and_is_dir :
        return None
    return p_test_dir

def get_defined_tests() :


    p_config_dir = vts_config_path()
    if not p_config_dir :
        return {}
    print("Looking for defined tests")

    p_test_dir = p_config_dir / "tests"
    if not p_test_dir.exists() :
        return {}
    test_dict = tests_from_test_dir(p_test_dir)
    return p_test_dir, test_dict

def tests_from_test_dir(p_test_config_dir = None) :

    if p_test_config_dir is None :
        return {}

    p_test_config_dir = Path(p_test_config_dir)

    exists_and_is_dir = p_test_config_dir.exists() and p_test_config_dir.is_dir()
    if not exists_and_is_dir :
        return {}
    test_config_files = list(p_test_config_dir.glob("test_config*.json"))
    test_dict = {}
    for ptest in test_config_files :
        with open(ptest, "r") as testfile :
            testdata = json.load(testfile)
        test_type = testdata["test_type"]
        test_dict[test_type] = ptest

    test_priority_list = {}
    test_priority_file = p_test_config_dir / "test_priority.json"
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
    return test_dict

def config_file_for_test(test_name = "", p_test_dir = None) :

    p_test_dir = Path(p_test_dir)
    exists_and_is_dir = p_test_dir.exists() and p_test_dir.is_dir()
    if not exists_and_is_dir :
        return None
    test_filename = "test_config_{}.json".format(test_name)
    p_test_filename = p_test_dir / test_filename
    if not p_test_filename.exists() :
        return None
    return p_test_filename
