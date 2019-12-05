#!/usr/bin/env python3

import sys, os
from pathlib import Path
import json

class VTSResultHandler :
    def __init__(self) :
        super(VTSResultHandler, self).__init__()
        self.test_info_dict = None

    def has_results(self) :
        return not self.test_info_dict is None

    def load_results(self, test_info = {}) :
        self.test_info_dict = test_info

    def clear(self) :
        self.test_info_dict = None

    def test_completion_status(self) :

        if not self.has_results() :
            return ""

        test_overview = self.test_info_dict["DATA"]
        return test_overview["TEST_COMPLETION"]

    def get_test_overview(self) :

        if not self.has_results() :
            return None
        return self.test_info_dict["DATA"]

    def get_result_info_dict(self) :

        if not self.has_results() :
            return None

        return self.test_info_dict["TEST_RESULTS"]

    def tests_complete(self) :

        if not self.has_results() :
            return False

        return self.test_completion_status() == "COMPLETE"

    def get_test_results(self) :

        test_results = self.get_result_info_dict()
        if not test_results :
            return None, None, None

        tests_succ, tests_pass, tests_fail = [], [], []

        for test_name, test_result_data in test_results.items() :
            test_result = test_result_data["RESULT"]
            if test_result == "SUCCESS" :
                tests_succ.append(test_name)
            elif test_result == "PASS" :
                tests_pass.append(test_name)
            elif test_result == "FAIL" :
                tests_fail.append(test_name)

        return tests_succ, tests_pass, tests_fail

    def get_result_data_for_test(self, test_name = "") :

        test_results = self.get_result_info_dict()
        if not test_results :
            return None
        if test_name not in test_results :
            return None
        for tname, test_result_data in test_results.items() :
            if tname == test_name :
                return test_result_data
        return None

    def final_test_result(self) :

        test_results = self.get_result_info_dict()
        if not test_results :
            return None, None

        tests_success, tests_pass, tests_fail = self.get_test_results()

        n_total_tests = len(tests_success) + len(tests_pass) + len(tests_fail)
        final_result = "SUCCESS"
        if len(tests_pass) > 0 :
            final_result = "PASS"
        if len(tests_fail) > 0 :
            final_result = "FAIL"
        if n_total_tests == 0 :
            final_result = "NONE"
        return final_result

    def result_summary_dict(self) :

        if not self.has_results() :
            return False

        test_metadata = self.get_test_overview()
        test_result_info = self.get_result_info_dict()

        vmm_sn = test_metadata["VMM_SERIAL_ID"]
        out_ext = test_metadata["TEST_OUTPUT_EXT"]

        out = {}
        result_summary = {}
        result_summary["VMM_SERIAL_ID"] = vmm_sn
        result_summary["OUTPUT_EXTENSION"] = out_ext
        result_summary["TESTS_COMPLETED"] = str(self.tests_complete())
        result_summary["FINAL_TEST_RESULT"] = str(self.final_test_result())

        tests_succ, test_pass, test_fail = self.get_test_results()
        all_tests_performed = tests_succ + test_pass + test_fail
        result_summary["TESTS_PERFORMED"] = all_tests_performed
        result_summary["TESTS_SUCCEEDED"] = tests_succ
        result_summary["TESTS_PASSED"] = test_pass
        result_summary["TESTS_FAILED"] = test_fail

        out["result_summary"] = result_summary

        # individual test data
        out["test_results"] = test_result_info

        return out

    def dump_results(self) :

        if not self.has_results() :
            return False

        test_metadata = self.get_test_overview()
        test_result_info = self.get_result_info_dict()

        ##
        ## construct the output name
        ##

        output_dir = test_metadata["TEST_OUTPUT_DIR"]
        tested_vmm_id = test_metadata["VMM_SERIAL_ID"]
        output_ext = test_metadata["TEST_OUTPUT_EXT"]
        output_filename = "vts_results_VMM%s" % tested_vmm_id
        if int(output_ext) > 0 :
            output_filename += "_%04d" % int(output_ext)
        output_filename += ".json"

        p_output = Path(output_dir)
        if not p_output.exists() or not p_output.is_dir() :
            print("ERROR Output directory for results (={}) not found - Saving results to current directory (={})".format(str(p_output), str(Path.cwd())))
            p_output = Path.cwd()

        p_output = p_output / output_filename

        ##
        ## prepare the output result data
        ##
        result_data = self.result_summary_dict()
        with open(p_output, "w") as ofile :
            json.dump(result_data, ofile)
