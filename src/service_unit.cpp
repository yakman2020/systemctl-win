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
#include <sstream>
#include "windows.h"
#include <vector>
#include <ios>
#include <limits>
#include <string>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <assert.h>
#include "service_unit.h"

using namespace std;


wchar_t BUFFER[MAX_BUFFER_SIZE] = { '\0' };

static class SystemDUnitPool the_pool;
class SystemDUnitPool *g_pool = &the_pool;

std::map<std::wstring, class SystemDUnit *> SystemDUnitPool::pool;

class SystemDUnit *
SystemDUnitPool::FindUnit(std::wstring name)

{   class SystemDUnit *&punit = pool[name];
    return punit;
}


class SystemDUnit *
SystemDUnitPool::ReadServiceUnit(std::wstring name, std::wstring service_unit_path) {
     wstring servicename = name;
     class SystemDUnit *punit = NULL;

     // Find the unit in the unit directory

     wifstream fs;
     try {
         fs.open(service_unit_path, std::fstream::in);
     }
     catch (exception e) {
         cerr << e.what();
     }
     if (fs.is_open()) {
         // wstring justname = servicename.substr(0, servicename.find_last_of('.'));
         punit = SystemDUnit::ParseSystemDServiceUnit(servicename, service_unit_path, fs);
     }
     else {
         cerr << L"No service unit " << servicename.c_str() << L"Found in unit library" << endl;
     }
     fs.close();

     return punit;
}

class SystemDUnit *SystemDUnit::ParseSystemDServiceUnit(wstring servicename, wstring unit_path, wifstream &fs)
{ 
    wcerr << L"ParseSystemDServiceUnit unit = " << servicename << std::endl;
   std::wstring line;
   class SystemDUnit *punit = new class SystemDUnit((wchar_t*)servicename.c_str(), unit_path.c_str());
 
   (void)fs.getline(BUFFER, MAX_BUFFER_SIZE);
   line = BUFFER;
    while (true) {
        
        if (fs.eof()) {
            break;
        }
     
        line.erase(
            std::remove_if(line.begin(), line.end(), 
                    [](wchar_t c) -> bool
                    { 
                        return std::isspace(c); 
                    }), 
                line.end());
        
        if (line[0] == ';' || line[0] == '#' ) {
             // Comment
            (void)fs.getline(BUFFER, MAX_BUFFER_SIZE);
            continue;
        }
        else if (line.compare(L"[Unit]") != wstring::npos) {
             // Then we need to parse the unit section
             wcerr << "parse unit section\n";
             line = punit->ParseUnitSection(fs);
        }
        else if (line.compare(L"[Service]") != wstring::npos) {
             // Then we need to parse the service section
             wcerr << "parse service section\n";
             line = punit->ParseServiceSection(fs);
        }
        else if (line.compare(L"[Install]") != wstring::npos) {
             // Then we need to parse the install section
             wcerr << "parse install section\n";
             line = punit->ParseInstallSection(fs);
        }
        else {
            if (line.length() == 0) {
                break;
            }
            wcerr << L"invalid section heading " << line.c_str() << endl;
            break;
        }
    }
   
    return punit;
}
 

// When it returns this will be located at EOF or the next section ([Service] for example).

static inline wstring split_elems(wifstream &fs, vector<wstring> &attrs, vector<wstring> &values)

{
    wstring attrname;
    wstring attrval;
    wstring line;

    while (true) {
        try { 
            if (fs.eof()) {
                break;
            }
            (void) fs.getline(BUFFER, MAX_BUFFER_SIZE);
        }
         catch (const std::exception &e) {
            wcerr << e.what() << endl;
            break;
        }

        line = BUFFER;
        int first_non_space = line.find_first_not_of(L" \t\r\n");
        if (first_non_space <= 0) {
            // blank line
            continue;
        }
        if (line[first_non_space] == '#' || line[first_non_space] == ';') {
            // This is a comment line. Treat it as empty
            continue;
        }

        // 2do: continuations via '\' at the end of the line.

        int split_pt  = line.find_first_of(L"=");
        if (split_pt == std::string::npos) {
            if (first_non_space == std::string::npos) {
                continue; // a blank line or eof
             }
             // If this line is part of the next section, we put it back
             if (line[first_non_space] == '[') {
                // fs.seekg(prev_pos, std::ios_base::beg);
             }
             break;
        }

        attrname = line.substr(first_non_space, split_pt);

        int last_non_space = line.find_last_not_of(L" \t\r\n");
        if (last_non_space == std::string::npos) {
            last_non_space = line.length();
        }

        attrval  = line.substr(split_pt+1, last_non_space );
        attrs.push_back(wstring(attrname));
        values.push_back(wstring(attrval));
        // wcerr << "attr = " << attrname.c_str() << endl;
    }
    return line;
}


static inline enum SystemDUnit::OUTPUT_TYPE String_To_OutputType(const wchar_t *str)

{
    wstring val = str;

    if (val.compare(L"inherit") == 0) {
        return SystemDUnit::OUTPUT_TYPE_INHERIT;
    }
    else if (val.compare(L"null") == 0) {
        return SystemDUnit::OUTPUT_TYPE_NULL;
    }
    else if (val.compare(L"tty") == 0) {
        return SystemDUnit::OUTPUT_TYPE_TTY;
    }
    else if (val.compare(L"journal") == 0) {
        return SystemDUnit::OUTPUT_TYPE_JOURNAL;
    }
    else if (val.compare(L"syslog") == 0) {
        return SystemDUnit::OUTPUT_TYPE_SYSLOG;
    }
    else if (val.compare(L"kmsg") == 0) {
        return SystemDUnit::OUTPUT_TYPE_KMSG;
    }
    else if (val.compare(L"journal+console") == 0) {
        return SystemDUnit::OUTPUT_TYPE_JOURNAL_PLUS_CONSOLE;
    }
    else if (val.compare(L"syslog+console") == 0) {
        return SystemDUnit::OUTPUT_TYPE_SYSLOG_PLUS_CONSOLE;
    }
    else if (val.compare(L"kmsg+console") == 0) {
        return SystemDUnit::OUTPUT_TYPE_KMSG_PLUS_CONSOLE;
    }
    else if (val.compare(0, 5, L"file:path") == 0) {
        return SystemDUnit::OUTPUT_TYPE_FILE;
    }
    else if (val.compare(L"socket") == 0) {
        return SystemDUnit::OUTPUT_TYPE_SOCKET;
    }
    else if (val.compare(0, 3, L"fd:name. ") == 0) {
        return SystemDUnit::OUTPUT_TYPE_FD;
    }
    else {
        return SystemDUnit::OUTPUT_TYPE_INVALID;
    }
}

wstring SystemDUnit::ParseUnitSection( wifstream &fs)

{ 
    vector<wstring> attrs;
    vector<wstring> values;

    wstring retval = split_elems(fs, attrs, values);
   
    for (auto i = 0; i < attrs.size(); i++) {
        if (attrs[i].compare(L"Description") == 0) {
            wcout << L"Description = " << values[i].c_str() << endl;
            this->description = values[i].c_str(); 
        }
        else if (attrs[i].compare(L"Documentation") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Requires") == 0) {
            wcout << "Requires " << values[i].c_str() << endl;

            wstring value_list = values[i]; 
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->requires.push_back(value_list.substr(start, end));
                }
                else {
                    this->requires.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"Requisite") == 0) {
            wcout << "Requisite " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->requisite.push_back(value_list.substr(start, end));
                }
                else {
                    this->requisite.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"Wants") == 0) {
            wcout << "Wants " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->wants.push_back(value_list.substr(start, end));
                }
                else {
                    this->wants.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"BindsTo") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PartOf") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Conflicts") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Before") == 0) {
            wcout << "Before " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->before.push_back(value_list.substr(start, end));
                }
                else {
                    this->before.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"After") == 0) {
            wcout << "After " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->after.push_back(value_list.substr(start, end));
                }
                else {
                    this->after.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"OnFailure") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PropagatesReloadTo") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ReloadPropagatedFrom") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"JoinsNamespaceOf") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RequiresMountsFor") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"OnFailureJobMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"IgnoreOnIsolate") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StopWhenUnneeded") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RefuseManualStart") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RefuseManualStop") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AllowIsolate") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"DefaultDependencies") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CollectMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"JobTimeoutSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"JobRunningTimeoutSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"JobTimeoutAction") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"JobTimeoutRebootArgument") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StartLimitIntervalSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StartLimitBurst") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StartLimitAction") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"FailureAction") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SuccessAction") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RebootArgument") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionArchitecture") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionVirtualization") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionHost") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionKernelCommandLine") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionKernelVersion") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionSecurity") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionCapability") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionACPower") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionNeedsUpdate") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionFirstBoot") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathExists") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathExistsGlob") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathIsDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathIsSymbolicLink") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathIsMountPoint") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionPathIsReadWrite") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionDirectoryNotEmpty") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionFileNotEmpty") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionFileIsExecutable") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionUser") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionGroup") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConditionControlGroupController") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertArchitecture") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertVirtualization") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertHost") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertKernelCommandLine") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertKernelVersion") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertSecurity") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertCapability") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertACPower") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertNeedsUpdate") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertFirstBoot") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathExists") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathExistsGlob") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathIsDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathIsSymbolicLink") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathIsMountPoint") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertPathIsReadWrite") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertDirectoryNotEmpty") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertFileNotEmpty") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertFileIsExecutable") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertUser") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertGroup") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AssertControlGroupController") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            wcerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }
    
    return retval;
}

wstring SystemDUnit::ParseServiceSection( wifstream &fs)

{   
    static const unsigned long  ATTRIBUTE_BIT_TYPE                       =  0x00000001;
    static const unsigned long  ATTRIBUTE_BIT_REMAIN_AFTER_EXIT          =  0x00000002;
    static const unsigned long  ATTRIBUTE_BIT_GUESS_MAIN_PID             =  0x00000004;
    static const unsigned long  ATTRIBUTE_BIT_PID_FILE                   =  0x00000008;
    static const unsigned long  ATTRIBUTE_BIT_BUS_NAME                   =  0x00000010;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START_PRE             =  0x00000020;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START                 =  0x00000040;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START_POST            =  0x00000080;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_RELOAD                =  0x00000100;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_STOP                  =  0x00000200;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_STOP_POST             =  0x00000400;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_SEC                =  0x00000800;
    static const unsigned long  ATTRIBUTE_BIT_TIMEOUT_START_SEC          =  0x00001000;
    static const unsigned long  ATTRIBUTE_BIT_TIMEOUT_STOP_SEC           =  0x00002000;
    static const unsigned long  ATTRIBUTE_BIT_RUNTIME_MAX_SEC            =  0x00008000;
    static const unsigned long  ATTRIBUTE_BIT_WATCHDOG_SEC               =  0x00010000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART                    =  0x00020000;
    static const unsigned long  ATTRIBUTE_BIT_SUCCESS_EXIT_STATUS        =  0x00040000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_PREVENT_EXIT_STATUS =  0x00080000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_FORCE_EXIT_STATUS  =  0x00100000;
    static const unsigned long  ATTRIBUTE_BIT_PERMISSIONS_START_ONLY     =  0x00200000;
    static const unsigned long  ATTRIBUTE_BIT_ROOT_DIRECTORY_START_ONLY  =  0x00400000;
    static const unsigned long  ATTRIBUTE_BIT_NON_BLOCKING               =  0x00800000;
    static const unsigned long  ATTRIBUTE_BIT_NOTIFY_ACCESS              =  0x01000000;
    static const unsigned long  ATTRIBUTE_BIT_SOCKETS                    =  0x02000000;
    static const unsigned long  ATTRIBUTE_BIT_FILE_DESCRIPTOR_STORE_MAX  =  0x04000000;
    static const unsigned long  ATTRIBUTE_BIT_USB_FUNCTION_DESCRIPTORS   =  0x08000000;
    static const unsigned long  ATTRIBUTE_BIT_USB_FUNCTION_STRINGS       =  0x10000000;

    unsigned long attr_bitmask = 0;
    vector<wstring> attrs;
    vector<wstring> values;

    enum ServiceType attr_type = SERVICE_TYPE_UNDEFINED;
    boolean attr_remainafterexit = false;
    boolean attr_guessmainpid    = true;
    wstring  attr_pidfile  = L"";
    wstring  attr_busname  = L"";
    vector<wstring> attr_execstart;

    wstring retval = split_elems(fs, attrs, values);

    for (auto i = 0; i < attrs.size(); i++) {

        if (attrs[i].compare(L"Type") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            
            if (values[i].compare(L"simple") == 0) {
                attr_type = SERVICE_TYPE_SIMPLE;
            }
            else if (values[i].compare(L"forking") == 0) {
                attr_type = SERVICE_TYPE_FORKING;
            }
            else if (values[i].compare(L"oneshot") == 0) {
                attr_type = SERVICE_TYPE_ONESHOT;
            }
            else if (values[i].compare(L"dbus") == 0) {
                attr_type = SERVICE_TYPE_DBUS;
            }
            else if (values[i].compare(L"notify") == 0) {
                attr_type = SERVICE_TYPE_NOTIFY;
            }
            else if (values[i].compare(L"idle") == 0) {
                attr_type = SERVICE_TYPE_IDLE;
            }
            else {
                attr_type = SERVICE_TYPE_UNDEFINED;
                wcerr << "service type " << values[i].c_str() << "is unknown" << endl;
            }
        }
        else if (attrs[i].compare(L"RemainAfterExit") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_REMAIN_AFTER_EXIT;
            if (values[i].compare(L"yes") == 0) {
                attr_remainafterexit = true;
            }
            else if (values[i].compare(L"no") == 0) {
                attr_remainafterexit = false;
            }
            else {
                attr_remainafterexit = false;
            }
            this->remain_after_exit = attr_remainafterexit;
        }
        else if (attrs[i].compare(L"GuessMainPID") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_GUESS_MAIN_PID;
            if (values[i].compare(L"yes") == 0) {
                attr_guessmainpid = true;
            }
            else if (values[i].compare(L"no") == 0) {
                attr_guessmainpid = false;
            }
            else {
                attr_guessmainpid = false;
            }
            this->guess_main_pid = attr_guessmainpid;
        }
        else if (attrs[i].compare(L"PIDFile") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_PID_FILE;
            this->pid_file = values[i];
        }
        else if (attrs[i].compare(L"BusName") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_BUS_NAME;
            this->bus_name = values[i];
        }
        else if (attrs[i].compare(L"ExecStartPre") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START_PRE;
            this->exec_start_pre.push_back(values[i]);
        }
        else if (attrs[i].compare(L"ExecStart") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START;
            this->exec_start.push_back(values[i]);
        }
        else if (attrs[i].compare(L"ExecStartPost") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START_POST;
            this->exec_start_post.push_back(values[i]);
        }
        else if (attrs[i].compare(L"ExecReload") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_RELOAD;
            this->exec_reload.push_back(values[i]);
        }
        else if (attrs[i].compare(L"ExecStop") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_STOP;
            this->exec_stop.push_back(values[i]);
        }
        else if (attrs[i].compare(L"ExecStopPost") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_STOP_POST;
            this->exec_stop_post.push_back(values[i]);
        }
        else if (attrs[i].compare(L"RestartSec") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_SEC;
            if (values[i].compare(L"infinity") == 0 ) {
                this->restart_sec = DBL_MAX;
            }
            else {
                try {
                   this->restart_sec = std::stod(values[i]);
                }
                catch (const std::exception &e) {
                    wcerr << "restart_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"TimeoutStartSec") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_TIMEOUT_START_SEC;
            if (values[i].compare(L"infinity") == 0 ) {
                this->timeout_start_sec = DBL_MAX;
            }
            else {
                try {
                   this->timeout_start_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    wcerr << "timeout_start_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"TimeoutStopSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_TIMEOUT_STOP_SEC;
            if (values[i].compare(L"infinity") == 0 ) {
                this->timeout_stop_sec = DBL_MAX;
            }
            else {
                try {
                    this->timeout_stop_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    wcerr << "timeout_stop_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"TimeoutSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= (ATTRIBUTE_BIT_TIMEOUT_START_SEC | ATTRIBUTE_BIT_TIMEOUT_STOP_SEC);
            if (values[i].compare(L"infinity") == 0) {
                this->timeout_start_sec = DBL_MAX;
                this->timeout_stop_sec = DBL_MAX;
            }
            else {
                try {
                   auto val = stod(values[i]);
                   this->timeout_stop_sec = val;
                   this->timeout_stop_sec = val;
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    wcerr << "timeout_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"RuntimeMaxSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RUNTIME_MAX_SEC;
            if (values[i].compare(L"infinity") == 0) {
                this->max_runtime_sec = DBL_MAX;
            }
            else {
                try {
                    this->max_runtime_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    wcerr << "RuntimeMaxSec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"WatchdogSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_WATCHDOG_SEC;
            if (values[i].compare(L"infinity") == 0 ) {
                this->watchdog_sec = DBL_MAX;
            }
            else {
                try {
                    this->watchdog_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    wcerr << "watchdog_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare(L"Restart") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART;
        }
        else if (attrs[i].compare(L"SuccessExitStatus") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_SUCCESS_EXIT_STATUS;
        }
        else if (attrs[i].compare(L"RestartPreventExitStatus") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_PREVENT_EXIT_STATUS;
        }
        else if (attrs[i].compare(L"RestartForceExitStatus") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_FORCE_EXIT_STATUS;
        }
        else if (attrs[i].compare(L"PermissionsStartOnly") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_PERMISSIONS_START_ONLY;
        }
        else if (attrs[i].compare(L"RootDirectoryStartOnly") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_ROOT_DIRECTORY_START_ONLY;
        }
        else if (attrs[i].compare(L"NonBlocking") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_NON_BLOCKING;
        }
        else if (attrs[i].compare(L"NotifyAccess") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_NOTIFY_ACCESS;
        }
        else if (attrs[i].compare(L"Sockets") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_SOCKETS;
        }
        else if (attrs[i].compare(L"FileDescriptorStoreMax") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_FILE_DESCRIPTOR_STORE_MAX;
        }
        else if (attrs[i].compare(L"USBFunctionDescriptors") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_USB_FUNCTION_DESCRIPTORS;
        }
        else if (attrs[i].compare(L"USBFunctionStrings") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_USB_FUNCTION_STRINGS;
        }
        else if (attrs[i].compare(L"WorkingDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RootDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RootImage") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"MountAPIVFS") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"BindPaths") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"BindReadOnlyPaths") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"User") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Group") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"DynamicUser") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SupplementaryGroups") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PAMName") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CapabilityBoundingSet") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AmbientCapabilities") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"NoNewPrivileges") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SecureBits") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SELinuxContext") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"AppArmorProfile") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SmackProcessLabel") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"UMask") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"KeyringMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"OOMScoreAdjust") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TimerSlackNSec") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Personality") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"IgnoreSIGPIPE") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Nice") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CPUSchedulingPolicy") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CPUSchedulingPriority") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CPUSchedulingResetOnFork") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CPUAffinity") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"IOSchedulingClass") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"IOSchedulingPriority") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ProtectSystem") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ProtectHome") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RuntimeDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StateDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CacheDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"LogsDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConfigurationDirectory") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RuntimeDirectoryMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StateDirectoryMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"CacheDirectoryMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"LogsDirectoryMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ConfigurationDirectoryMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RuntimeDirectoryPreserve") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ReadWritePaths") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ReadOnlyPaths") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"InaccessiblePaths") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TemporaryFileSystem") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PrivateTmp") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PrivateDevices") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PrivateNetwork") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"PrivateUsers") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ProtectKernelTunables") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ProtectKernelModules") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"ProtectControlGroups") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RestrictAddressFamilies") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RestrictNamespaces") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"LockPersonality") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"MemoryDenyWriteExecute") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RestrictRealtime") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"RemoveIPC") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"MountFlags") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SystemCallFilter") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SystemCallErrorNumber") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SystemCallArchitectures") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"Environment") == 0) {
            wcout << "Environment " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->environment_vars.push_back(value_list.substr(start, end));
                }
                else {
                    this->environment_vars.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"EnvironmentFile") == 0) {
            wcout << "EnvironmentFile= " << values[i].c_str() << endl;

            this->environment_file.push_back(values[i].c_str());
        }
        else if (attrs[i].compare(L"PassEnvironment") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"UnsetEnvironment") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StandardInput") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StandardOutput") == 0) {
            wcout << "StandardOutput = " << values[i].c_str() << endl;
            enum OUTPUT_TYPE out_type = String_To_OutputType(values[i].c_str());
            switch (out_type) {
            default:
            case OUTPUT_TYPE_INVALID:
                wcerr << "StandardOutput invalid value " << values[i].c_str() << endl;
                break;
            case OUTPUT_TYPE_INHERIT:
            case OUTPUT_TYPE_NULL:
            case OUTPUT_TYPE_TTY:
            case OUTPUT_TYPE_JOURNAL:
            case OUTPUT_TYPE_SYSLOG:
            case OUTPUT_TYPE_KMSG:
            case OUTPUT_TYPE_JOURNAL_PLUS_CONSOLE:
            case OUTPUT_TYPE_SYSLOG_PLUS_CONSOLE:
            case OUTPUT_TYPE_KMSG_PLUS_CONSOLE:
            case OUTPUT_TYPE_SOCKET:
                this->output_type = out_type;
                break;
            case OUTPUT_TYPE_FILE:   // requires a path
            case OUTPUT_TYPE_FD:      // requires a name
                int    delpos = values[i].find_first_of(L":")+1;
                wstring    val = values[i].substr(delpos, values[i].length()-delpos);
                this->output_type = out_type;
                this->output_file_path = val;
                break;
            }
        }
        else if (attrs[i].compare(L"StandardError") == 0) {
            wcout << "StandardError = " << values[i].c_str() << endl;
            enum OUTPUT_TYPE out_type = String_To_OutputType(values[i].c_str());
            switch (out_type) {
            default:
            case OUTPUT_TYPE_INVALID:
                wcerr << "StandardError invalid value " << values[i].c_str() << endl;
                break;
            case OUTPUT_TYPE_INHERIT:
            case OUTPUT_TYPE_NULL:
            case OUTPUT_TYPE_TTY:
            case OUTPUT_TYPE_JOURNAL:
            case OUTPUT_TYPE_SYSLOG:
            case OUTPUT_TYPE_KMSG:
            case OUTPUT_TYPE_JOURNAL_PLUS_CONSOLE:
            case OUTPUT_TYPE_SYSLOG_PLUS_CONSOLE:
            case OUTPUT_TYPE_KMSG_PLUS_CONSOLE:
            case OUTPUT_TYPE_SOCKET:
                this->output_type = out_type;
                break;
            case OUTPUT_TYPE_FILE:   // requires a path
            case OUTPUT_TYPE_FD:      // requires a name
                int    delpos = values[i].find_first_of(L":")+1;
                wstring    val = values[i].substr(delpos, values[i].length()-delpos);
                this->error_type = out_type;
                this->error_file_path = val;
                break;
            }
        }
        else if (attrs[i].compare(L"StandardInputText") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"StandardInputData") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"LogLevelMax") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"LogExtraFields") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SyslogIdentifier") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SyslogFacility") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SyslogLevel") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"SyslogLevelPrefix") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TTYPath") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TTYReset") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TTYVHangup") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"TTYVTDisallocate") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"UtmpIdentifier") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"UtmpMode") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            wcerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }

    // if type is not set, then we have default value combinatiosn that
    // produce a type value. We behvae after as if they had been specified that way

    if (! (attr_bitmask & ATTRIBUTE_BIT_TYPE)) {
        
        if ((attr_bitmask & ATTRIBUTE_BIT_EXEC_START) ) {
            if (! (attr_bitmask & ATTRIBUTE_BIT_BUS_NAME) ) {
                attr_type = SERVICE_TYPE_SIMPLE;
                attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            }
            else {
                attr_type = SERVICE_TYPE_DBUS;
                attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            }
        }
        else {
            attr_type = SERVICE_TYPE_ONESHOT;
            attr_bitmask |= ATTRIBUTE_BIT_TYPE;
        }
    }

    // Validate the combinations of definitions


    // Set the values if value
    this->service_type = attr_type;

    return retval;
}

wstring SystemDUnit::ParseInstallSection( wifstream &fs)

{
    vector<wstring> attrs;
    vector<wstring> values;

    wstring retval = split_elems(fs, attrs, values);
    for (auto i = 0; i < attrs.size(); i++) {
        if (attrs[i].compare(L"Alias") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"WantedBy") == 0) {
            wcout << "WantedBy " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->wanted_by.push_back(value_list.substr(start, end));
                }
                else {
                    this->wanted_by.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"RequiredBy") == 0) {
            wcout << "RequiredBy " << values[i].c_str() << endl;

            wstring value_list = values[i];
            int end = 0;
            for (auto start = 0; start != std::string::npos; start = end) {
                end = value_list.find_first_of(' ', start);
                if (end != string::npos){
                    this->required_by.push_back(value_list.substr(start, end));
                }
                else {
                    this->required_by.push_back(value_list);
                }
            }
        }
        else if (attrs[i].compare(L"Also") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare(L"DefaultInstance") == 0) {
            wcout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            wcerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }
    return retval;
}


boolean 
SystemDUnitPool::DirExists(wstring dir_path)

{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW ffd;
    LARGE_INTEGER filesize;
   
    wstring dos_path = dir_path + L"/*";
    std::replace_if(dos_path.begin(), dos_path.end(),
            [](wchar_t c) -> bool
                {
                    return c == '/';
                }, '\\');
    
    hFind = FindFirstFileW(dos_path.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) 
    {
       return false;
    } 
    FindClose(hFind);
    return true;
}

boolean 
SystemDUnitPool::Apply(wstring dir_path, boolean (*action)(wstring dir_path, void *context ), void *context)

{
    boolean rslt = false;

#if VC_SUPPORTS_STD_FILESYSTEM
    for ( auto & thisfile : std::filesystem::directory_iterator(SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH)) {
         std::wcout << thisfile << endl;
    }
#else
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW ffd;
    LARGE_INTEGER filesize;
   
    wstring dos_path = dir_path + L"/*";
    std::replace_if(dos_path.begin(), dos_path.end(),
            [](wchar_t c) -> bool
                {
                    return c == '/';
                }, '\\');
    
    DWORD errval = 0;
    hFind = FindFirstFileW(dos_path.c_str(), &ffd);
 
    if (INVALID_HANDLE_VALUE == hFind) 
    {
        errval = GetLastError();
        wcerr << L"Could not find directory path " << dir_path.c_str() << L" error = " << errval << endl;
 
       return false;
    } 
    
    // List all the files in the directory with some info about them.
 
    do
    {
       if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
       {
           wstring subpath = ffd.cFileName;

           if ( (subpath.compare(L".")) != 0 && ( subpath.compare(L"..") != 0)) {
               wcerr << "subdir found " << subpath.c_str() << endl;
               subpath = dir_path + L"/" + subpath;
               rslt = Apply(subpath, action, context);
               wcerr << "return from dir" << subpath.c_str() << endl;
           }
           
       }
       else
       {
          filesize.LowPart = ffd.nFileSizeLow;
          filesize.HighPart = ffd.nFileSizeHigh;
          wcerr << L"filename " << ffd.cFileName << L" file size " << filesize.QuadPart << endl;
          if (action) {
              rslt = (*action)(dir_path+L"/"+ffd.cFileName, context);
          }
       }
    }
    while (FindNextFileW(hFind, &ffd) != 0);
  
    FindClose(hFind);
#endif
    return rslt;
}

// Returns true if the file is read in or ignored. false if read fails
static boolean read_unit(wstring file_path, void *context)

{
    wstring servicename = file_path.substr(file_path.find_last_of('/') + 1);
    wstring file_type = file_path.substr(file_path.find_last_of('.') + 1);

    if ((file_type.compare(L"service") == 0) ||
        (file_type.compare(L".target") == 0) ||
        (file_type.compare(L".timer") == 0) ||
        (file_type.compare(L".socket") == 0)) {
        SystemDUnit *punit = SystemDUnitPool::ReadServiceUnit(servicename, file_path);
        if (!punit) {
            // Complain and exit
            wcerr << "Failed to load unit: Unit file " << file_path.c_str() << "is invalid\n";
            return false;
        }
        else {
            punit->Enable(true);
        }
    }
    return true;
}


static boolean
enable_required_unit(wstring file_path, void *context )

{
    wcerr << L"enable required Unit " << file_path.c_str() << std::endl;
    return true;
}

static boolean
enable_wanted_unit(wstring file_path, void *context )

{ 
    boolean unit_loaded = false;
    wstring servicename = file_path.substr(file_path.find_last_of('/') + 1);

    wcerr << L"enable wanted Unit " << file_path.c_str() << std::endl;

    // normalise the file path
    wstring dos_path = file_path; // Make a copy
    std::replace_if(dos_path.begin(), dos_path.end(),
            [](wchar_t c) -> bool
                {
                    return c == '/';
                }, '\\');
    
    unit_loaded = read_unit(dos_path, context);
    class  SystemDUnit *punit = NULL;
    if (unit_loaded) {
       punit = SystemDUnitPool::FindUnit(servicename);
        if (!punit) {
            wcerr << L"could not find unit " << servicename << std::endl;
            return false;
        }
    }
    else {
        wcerr << L"could not find unit " << servicename << std::endl;
        return false;
    }

    assert(punit != NULL);
    class SystemDUnit *parent_unit = (class SystemDUnit *)context;
    assert(parent_unit != NULL);

    parent_unit->AddWanted(punit->Name());

    return true;
}

boolean 
SystemDUnit::Enable(boolean block)

{
    wchar_t * buffer;
    wstring servicename = this->name;

    // Is the active dir there? If not, create it.
    wstring active_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH;
    if (!SystemDUnitPool::DirExists(active_dir_path)) {
         if (!CreateDirectoryW(active_dir_path.c_str(), NULL)) {  // 2do: security attributes
             wcerr << L"Cold not create active directory" << std::endl;
         }
    }

    // Is there a requires directory?
    wstring requires_dir_path = SystemDUnitPool::UNIT_DIRECTORY_PATH+L"/"+servicename+L".requires";
    if (SystemDUnitPool::DirExists(requires_dir_path)) {
        wstring active_requires_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".requires";

        (void)CreateDirectoryW(active_requires_dir_path.c_str(), NULL);
        // Enable all of the units and add to the requires list
        
        (void)SystemDUnitPool::Apply(requires_dir_path, enable_required_unit, (void*)this);
    }

    // Is there a wants directory?
    wstring wants_dir_path = SystemDUnitPool::UNIT_DIRECTORY_PATH+L"/"+servicename+L".wants";
    if (SystemDUnitPool::DirExists(wants_dir_path)) {
        wstring active_wants_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".wants";
        (void)CreateDirectoryW(active_wants_dir_path.c_str(), NULL);

        // Enable all of the units and add to the wants list
        (void)SystemDUnitPool::Apply(wants_dir_path, enable_wanted_unit, (void*)this);
    }

    // Enable all of the units and add to the requires list


    if (this->IsEnabled()) {
        // We don't error but we don't do anything
        return true;
    }

    // Even if we aren't running now, we could have a unit in the active directory
    // If so, we just need to register ourseleves with the service manager
    wifstream checkfs(SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename);
    if (checkfs.is_open()) {
        checkfs.close();
        this->RegisterService();
        this->is_enabled = true;
        return true;
    }
    else {
              
        // If there is no file in the active directory, we copy one over, then register.

        wstring service_unit_path = SystemDUnitPool::UNIT_DIRECTORY_PATH+L"/"+servicename;
    
        // Find the unit in the unit library
        wifstream fs(service_unit_path, std::fstream::in);
        if (!fs.is_open()) {
             wcerr << "No service unit " << servicename.c_str() << "Found in unit library" << endl;
             return false;
        }
        fs.seekg (0, fs.end);
        int length = fs.tellg();
        fs.seekg (0, fs.beg);
        buffer = new wchar_t [length];
        fs.read (buffer,length);
    
        fs.close();
    
        wofstream ofs(SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename);
        ofs.write (buffer,length);
        ofs.close();
    
        this->RegisterService();
    
        // and mark the object
        this->is_enabled = true;
     
        return true;
    }
}

static boolean
disable_required_unit(wstring file_path, void *context )

{
    wcerr << L"disable required Unit " << file_path.c_str() << std::endl;
    return true;
}

static boolean
disable_wanted_unit(wstring file_path, void *context )

{
    wcerr << L"disable wanted Unit " << file_path.c_str() << std::endl;
    return true;
}

boolean SystemDUnit::Disable(boolean block)

{
    wstring servicename = this->name;

    // Is there a requires directory?
    wstring requires_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".requires";
    if (SystemDUnitPool::DirExists(requires_dir_path)) {
        // Enable all of the units and add to the requires list
        (void)SystemDUnitPool::Apply(requires_dir_path, disable_required_unit, (void*)this);
    }

    // Is there a wants directory?
    wstring wants_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".wants";
    if (SystemDUnitPool::DirExists(wants_dir_path)) {
        // Enable all of the units and add to the wants list
        (void)SystemDUnitPool::Apply(wants_dir_path, disable_wanted_unit, (void*)this);
    }

    // Disable unregisters the service from the service manager, but leaves the service unit 
    // in place. The next daemon-reload will pick it up again

    this->UnregisterService();
    
    // and mark the object
    this->is_enabled = false;
    return false;
}


static boolean
mask_required_unit(wstring file_path, void *context )

{
    wcerr << L"mask required Unit " << file_path.c_str() << std::endl;
    return true;
}

static boolean
mask_wanted_unit(wstring file_path, void *context )

{
    wcerr << L"mask wanted Unit " << file_path.c_str() << std::endl;
    return true;
}

boolean SystemDUnit::Mask(boolean block)

{
    // Mask unregisters the service from the service manager, then deletes the service unit 
    // from the active directory.
    wstring servicename = this->name;

    // Is there a requires directory?
    wstring requires_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".requires";
    if (SystemDUnitPool::DirExists(requires_dir_path)) {
        // Enable all of the units and add to the requires list
        (void)SystemDUnitPool::Apply(requires_dir_path, mask_required_unit, (void*)this);
    }

    // Is there a wants directory?
    wstring wants_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".wants";
    if (SystemDUnitPool::DirExists(wants_dir_path)) {
        // Enable all of the units and add to the wants list
        (void)SystemDUnitPool::Apply(wants_dir_path, mask_wanted_unit, (void*)this);
    }

    this->UnregisterService();

    std::wstring filepath_W = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename;
    std::string filepath_A = std::string(filepath_W.begin(), filepath_W.end());
    // Delete the file
    std::remove(filepath_A.c_str());
    
    // and mark the object
    this->is_enabled = false;
    return false;
}



static boolean
unmask_required_unit(wstring file_path, void *context )

{
    wcerr << L"unmask required Unit " << file_path.c_str() << std::endl;
    return true;
}

static boolean
unmask_wanted_unit(wstring file_path, void *context )

{
    wcerr << L"unmask wanted Unit " << file_path.c_str() << std::endl;
    return true;
}

boolean SystemDUnit::Unmask(boolean block)

{
    wchar_t * buffer;

    wstring servicename = this->name;
    // Is there a requires directory?
    wstring requires_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".requires";
    if (SystemDUnitPool::DirExists(requires_dir_path)) {
        // Enable all of the units and add to the requires list
        (void)SystemDUnitPool::Apply(requires_dir_path, unmask_required_unit, (void*)this);
    }

    // Is there a wants directory?
    wstring wants_dir_path = SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename+L".wants";
    if (SystemDUnitPool::DirExists(wants_dir_path)) {
        // Enable all of the units and add to the wants list
        (void)SystemDUnitPool::Apply(wants_dir_path, unmask_wanted_unit, (void*)this);
    }

    // Is there a wants directory?
    if (this->IsEnabled()) {
        // We don't error but we don't do anything
        return true;
    }


    // Even if we aren't running now, we could have a unit in the active directory
    // If so, we just need to register ourseleves with the service manager
    wifstream checkfs(SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename);
    if (checkfs.is_open()) {
        checkfs.close();
        return true;
    }
    else {
              
        // If there is no file in the active directory, we copy one over, then register.

        wstring service_unit_path = SystemDUnitPool::UNIT_DIRECTORY_PATH+L"/"+servicename;
    
        // Find the unit in the unit library
        wifstream fs(service_unit_path, std::fstream::in);
        if (!fs.is_open()) {
             wcerr << "No service unit " << servicename.c_str() << "Found in unit library" << endl;
             return false;
        }
        fs.seekg (0, fs.end);
        int length = fs.tellg();
        fs.seekg (0, fs.beg);
        buffer = new wchar_t [length];
        fs.read (buffer,length);
    
        fs.close();
    
        wofstream ofs(SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH+L"/"+servicename);
        ofs.write (buffer,length);
        ofs.close();
    
        return true;
    }
    return false;
}




static void setup_before(SystemDUnit *punit, wstring const &before) 

{
    class SystemDUnit *pother_unit = SystemDUnitPool::FindUnit(before);

    // Add to the after list 
    


}


static void register_unit(std::pair<std::wstring, class SystemDUnit *> entry)

{
    class SystemDUnit *punit = entry.second;

    punit->RegisterService();
}



static void query_register_unit(std::pair<std::wstring, class SystemDUnit *> entry)
 
{
    class SystemDUnit *punit = entry.second;

    // Is the service loaded? Load it if not
    if (punit) {
        if (!punit->IsEnabled()) {
            punit->RegisterService();
        }
    }
}

void setup_own_dependencies(std::pair<std::wstring, class SystemDUnit *> entry)
 
{
    class SystemDUnit *punit = entry.second;

    if (punit) {
        for(auto other_service: punit->GetAfter()) {
            class SystemDUnit *pother_unit = g_pool->GetPool()[other_service];
            if (pother_unit) {
                punit->AddWaitDependency(pother_unit);
            }
        }

        for(auto other_service : punit->GetRequires()) {
            class SystemDUnit *pother_unit = g_pool->GetPool()[other_service];
            if (pother_unit) {
                punit->AddStartDependency(pother_unit);
            }
        }
    }
}


void setup_other_dependencies(std::pair<std::wstring, class SystemDUnit *> entry)
 
{
    class SystemDUnit *punit = entry.second;
    // where afters are our concern, we must setup before dependencies in the subject services..
    // So we add our befores to those services afters.

    if (punit) {
        for(auto other_service: punit->GetBefore()) {
            class SystemDUnit *pother_unit = g_pool->GetPool()[other_service];
            if (pother_unit) {
                pother_unit->AddWaitDependency(punit);
            }
        }

        for(auto other_service : punit->GetRequiredBy()) {
            class SystemDUnit *pother_unit = g_pool->GetPool()[other_service];
            if (pother_unit) {
                pother_unit->AddStartDependency(punit);
            }
        }
    }
}



void SystemDUnitPool::ReloadPool()

{
    wcerr << "do daemon reload" << endl;
    (void)Apply(SystemDUnitPool::UNIT_DIRECTORY_PATH.c_str(), read_unit, (void*)this);

     // Now we have the graph, we need to resolve a dependencies graph.
     for_each(g_pool->pool.begin(), g_pool->pool.end(), setup_own_dependencies);
     for_each(g_pool->pool.begin(), g_pool->pool.end(), setup_other_dependencies);
     for_each(g_pool->pool.begin(), g_pool->pool.end(), register_unit);
}


void SystemDUnitPool::LoadPool()

{
    wcerr << "do daemon reload" << endl;
    (void)Apply(SystemDUnitPool::UNIT_DIRECTORY_PATH.c_str(), read_unit, (void*)this);
for (auto member: g_pool->GetPool()) {
wcerr << L"key = " << member.first << "value = " << member.second->Name() << std::endl;
}

     // Now we have the graph, we need to resolve a dependencies graph.
     for_each(g_pool->pool.begin(), g_pool->pool.end(), setup_own_dependencies);
     for_each(g_pool->pool.begin(), g_pool->pool.end(), setup_other_dependencies);
     for_each(g_pool->pool.begin(), g_pool->pool.end(), query_register_unit);
}


