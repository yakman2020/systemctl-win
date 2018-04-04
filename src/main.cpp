/*
 * **==============================================================================
 * **
 * ** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
 * ** for license information.
 * **
 * **==============================================================================
 * */
#include <iostream>
#include <fstream>
#include "windows.h"
#include "service_unit.h"

char usage[] = "usage: "\
               "  systemctl start <servicename>\n" \
               "  systemctl start <servicename>\n";

using namespace std;


//char BUFFER[MAX_BUFFER_SIZE] = { '\0' };
string SystemDUnitPool::UNIT_DIRECTORY_PATH = ""; // Quasi constant
string SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH = ""; // Quasi constant

class SystemDUnitPool *g_pool = new class SystemDUnitPool; // Singleton

void do_daemon_reload()

{
  cerr << "do daemon reload" << endl;
}

int main(int argc, char *argv[])

{ 
    int argidx = 0;
    char *thisarg = NULL;
    std::cout << usage;

    for (argidx = 1, thisarg = argv[argidx]; thisarg != NULL; thisarg = argv[++argidx] ) {
        string argstr = thisarg;

        if (argstr.compare("daemon-reload") == 0) {
            do_daemon_reload();
        }
        else if (argstr.compare("enable") == 0) {
            thisarg = argv[++argidx] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            class SystemDUnit *unit = SystemDUnitPool::FindUnit(argstr);
            if (!unit) {
                // Then we need to create a unit record.
                unit = SystemDUnitPool::ReadServiceUnit(argstr);
                if (!unit) {
                    // Complain and exit
                    cerr << "Failed to enable unit: Unit file " << argstr.c_str() << "does not exist\n";
                    cerr << usage;
                    exit(1);
                }
                unit->Enable(false); // We will add non-blocking later
            }
        }
        else if (argstr.compare("disable") == 0) {
            thisarg = argv[++argidx] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            SystemDUnit *unit = SystemDUnitPool::FindUnit(argstr);
            if (!unit) {
                    // Complain and exit
                    cerr << "Failed to disable unit: Unit file " << argstr.c_str() << "does not exist\n";
                    cerr << usage;
                    exit(1);
            }
            unit->Disable(false);
        }
        else if (argstr.compare("is-enabled") == 0) {
            thisarg = argv[++argidx] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            SystemDUnit *unit = SystemDUnitPool::FindUnit(argstr);
            if (!unit) {
                    // Complain and exit
                    cerr << "Failed to disable unit: Unit file " << argstr.c_str() << "does not exist\n";
                    cerr << usage;
                    exit(1);
            }
            if (unit->IsEnabled() ) {
            }
            else {
            }
        }
        else if (argstr.compare("kill") == 0) {
        }
        else if (argstr.compare("mask") == 0) {
        }
        else if (argstr.compare("start") == 0) {
        }
        else if (argstr.compare("stop") == 0) {
        }
        else if (argstr.compare("restart") == 0) {
        }
        else if (argstr.compare("list-unit-files") == 0) {
        }
        else if (argstr.compare("show") == 0) {
        }
        else if (argstr.compare("status") == 0) {
        }
        else if (argstr.compare("set-default") == 0) {
        }
        else {
            if (!thisarg) {
                 // Complain and exit
                 cerr << "Unrecognised option\n";
                 cerr << usage;
                 exit(1);
            }
        }
    }
    exit(0);
}


