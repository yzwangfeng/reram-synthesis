/*
 * abc.h
 *
 * invoke abc
 */

#ifndef ABC_H_
#define ABC_H_

#include <iostream>
#include <fstream>
#include <map>
using namespace std;

#define ABC_LIB "genlib/NOR2.genlib"

extern "C" void Abc_Start();
extern "C" void Abc_Stop();
extern "C" void * Abc_FrameGetGlobalFrame();
extern "C" int Cmd_CommandExecute(void * pAbc, char * sCommand);

void abc_check_equivalence(string in_file, string out_file);                        // equivalence check
void abc_synthesize(string in_file, string write_cmd, string out_file);             // logic synthesis
void abc_map(string in_file, string write_cmd, string out_file, string lib);        // standard cell mapping
void abc_lutpack(string in_file, string write_cmd, string out_file, string lib);    // LUT mapping

#endif /* ABC_H_ */
