#include <cstring>
#include "abc.h"

map<string, string> alias;

void abc_initialize() {
    alias["b"] = "balance;";
    alias["rw"] = "rewrite;";
    alias["rwz"] = "rewrite -z;";
    alias["rf"] = "refactor;";
    alias["rfz"] = "refactor -z;";
    alias["resyn"] = alias["b"] + alias["rw"] + alias["rwz"] + alias["b"] +
                     alias["rwz"] + alias["b"];
    alias["resyn2"] = alias["b"] + alias["rw"] + alias["rf"] + alias["b"] +
                      alias["rw"] + alias["rwz"] + alias["b"] + alias["rfz"] +
                      alias["rwz"] + alias["b"];
    alias["choice"] = "fraig_store;" + alias["resyn"] + "fraig_store;" +
                      alias["resyn2"] + "fraig_store;" + "fraig_restore;";
}

void execute_command(string s) {
    void *pAbc;
    char command[10000];
    strcpy(command, s.c_str());

    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
    if (Cmd_CommandExecute(pAbc, command)) {
        fprintf(stdout, "Cannot execute command \"%s\".\n", command);
    }
    Abc_Stop();
}

void abc_check_equivalence(string in_file, string out_file) {
    abc_initialize();
    string command = "cec " + in_file + " " + out_file + ";";
    execute_command(command);
}

void abc_synthesize(string in_file, string write_cmd, string out_file) {
    abc_initialize();
    string command = "read " + in_file + ";";
    command += alias["resyn"] + alias["resyn2"];
    command += write_cmd + " " + out_file + ";";
    execute_command(command);
}

void abc_map(string in_file, string write_cmd, string out_file, string lib) {
    abc_initialize();
    string command = "read_library " + lib + ";";
    command += "read " + in_file + ";";
    command += alias["resyn"] + alias["resyn2"];
    command += "map;";
    command += write_cmd + " " + out_file + ";";
    execute_command(command);
    cout << "Standard cell mapping done!" << endl;
}

void abc_lutpack(string in_file, string write_cmd, string out_file,
                 string lib) {
    abc_initialize();
    string command = "read_lut " + lib + ";";
    command += "read " + in_file + ";";
    command += alias["resyn"] + alias["resyn2"];
    command += "if;";
    for (int i = 0; i < 4; ++i) {
        command += alias["choice"] + "if;mfs;";
    }
    for (int i = 0; i < 2; ++i) {
        command += "lutpack;";
    }
    command += write_cmd + " " + out_file + ";";
    execute_command(command);
    cout << "LUT packing done!" << endl;
}
