#pragma once
#ifndef REMAC_TESTMAIN
#define REMAC_TESTMAIN 1

#include <string>

static unsigned int TOTAL_TESTS = 0;
static unsigned int PASSED_TESTS = 0;

void test_module(std::string name);

void test_condition(bool condition);

void test_main();

#endif // REMAC_TESTMAIN
