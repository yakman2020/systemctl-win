/*
 * **==============================================================================
 * **
 * ** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
 * ** for license information.
 * **
 * **==============================================================================
 * */
#include <iostream>
#include "popt.h"
#include "windows.h"

char usage[] = "usage: "\
               "  systemctl start <servicename>\n" \
               "  systemctl start <servicename>\n";

using namespace std;


class SystemDUnitPool {
public:
    SystemDUnitPool() { };
    ~SystemDUnitPool() { };
 
private:
   vector<SystemDUnit *>pool;
};


class SystemDUnit {
public:
    SystemDUnit(char *name) {
         this->name = name;
         this->dependencies = NULL;
         this->is_enabled = false;
        };
    ~SystemDUnit() { delete this->name; };

    boolean Start(boolean block) {  };
    boolean Stop(boolean block) {  };
    boolean Enable(boolean block) {  };
    boolean Disable(boolean block) {  };
    boolean IsEnabled() {  };
    void SetDependency(SystemDUnit *dependency; )
    vector<class SystemDUnit> GetDependency( return dependencies; )
 

    static SystemDUnitPool &UnitPool() { return pool; };
private:
    string name;
    boolean is_enabled;
    vector<class SystemDUnit *> dependencies;
    static SystemDUnitPool pool;
};


int main(int argc, char *argv[])

{ 
    char *thisarg = NULL;
    std::cout << usage;

    for (int i = 0; thisarg = argv[i]; thisarg != NULL; thisarg = argv[++i] ) {
        string argstr = thisarg;

        if (argstr.compare("daemon-reload") == 0) {
            do_daemon_reload()
        }
        else if (argstr.compare("enable") == 0) {
            thisarg = argv[++i] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            SystemDUnit *unit = SystemDUnit.GetPool().FindUnit(argstr);
            if (!unit) {
                // Then we need to create a unit record.
                unit = SystemDUnit.GetPool().ReadServiceUnit(argstr);
                if (!unit) {
                    // Complain and exit
                    cerr << "Failed to enable unit: Unit file " << argstr << "does not exist\n";
                    cerr << usage;
                    exit(1);
                }
                unit->Enable(false); // We will add non-blocking later
            }
        }
        else if (argstr.compare("disable")) {
            thisarg = argv[++i] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            SystemDUnit *unit = SystemDUnit.GetPool().FindUnit(argstr);
            if (!unit) {
                    // Complain and exit
                    cerr << "Failed to disable unit: Unit file " << argstr << "does not exist\n";
                    cerr << usage;
                    exit(1);
            }
            unit->Disable(false);
        }
        else if (argstr.compare("is-enabled")) {
            thisarg = argv[++i] ;
            if (!thisarg) {
                 // Complain and exit
                 cerr << "No unit specified\n";
                 cerr << usage;
                 exit(1);
            }
            argstr = thisarg; 

            SystemDUnit *unit = SystemDUnit.GetPool().FindUnit(argstr);
            if (!unit) {
                    // Complain and exit
                    cerr << "Failed to disable unit: Unit file " << argstr << "does not exist\n";
                    cerr << usage;
                    exit(1);
            }
            if (unit->IsEnabled() ) {
            }
            else {
            }
        }
        else if (argstr.compare("kill")) {
        }
        else if (argstr.compare("mask")) {
        };
        else if (argstr.compare("stop")) {
        }
        else if (argstr.compare("restart")) {
        }
        else if (argstr.compare("list-unit-files")) {
        }
        else if (argstr.compare("show")) {
        }
        else if (argstr.compare("status")) {
        }
        else if (argstr.compare("set-default")) {
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


