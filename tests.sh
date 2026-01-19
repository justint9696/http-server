#!/usr/bin/env bash

unit_tests=$(find "bin/test" -type f -name "*.out")
sh -c "bin/unit_tester $unit_tests"
