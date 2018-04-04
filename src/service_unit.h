/*
 * **==============================================================================
 * **
 * ** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
 * ** for license information.
 * **
 * **==============================================================================
 * */
#ifndef __WIN_SYSTEMD_SERVICE_UNIT_H__
#define __WIN_SYSTEMD_SERVICE_UNIT_H__
#include <iostream>
#include <fstream>
#include "windows.h"
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

#define MAX_BUFFER_SIZE 4096
extern char BUFFER[];

/* Exec action prefix characters:
 *
 * Prefix   Effect
 * "@"	If the executable path is prefixed with "@", the second specified token will be passed 
 *      as "argv[0]" to the executed process (instead of the actual filename), followed by the 
 *      further arguments specified.
 * "-"	If the executable path is prefixed with "-", an exit code of the command normally considered 
 *      a failure (i.e. non-zero exit status or abnormal exit due to signal) is ignored and considered success.
 * "+"	If the executable path is prefixed with "+" then the process is executed with full privileges.
        In this mode privilege restrictions configured with User=, Group=, CapabilityBoundingSet= or the 
 *      various file system namespacing options (such as PrivateDevices=, PrivateTmp=) are not applied to
 *      the invoked command line (but still affect any other ExecStart=, ExecStop=).
 * "!"	Similar to the "+" character discussed above this permits invoking command lines with elevated privileges.
 *      However, unlike "+" the "!" character exclusively alters the effect of User=, Group= and SupplementaryGroups=,
 *      i.e. only the stanzas the affect user and group credentials. Note that this setting may be combined
 *      with DynamicUser=, in which case a dynamic user/group pair is allocated before the command is invoked,
 *      but credential changing is left to the executed process itself.
 *  
 * "!!"	This prefix is very similar to "!", however it only has an effect on systems lacking support for ambient
 *      process capabilities, i.e. without support for AmbientCapabilities=. It's intended to be used for unit
 *      files that take benefit of ambient capabilities to run processes with minimal privileges wherever possible
 *      while remaining compatible with systems that lack ambient capabilities support. Note that when "!!" is used,
 *      and a system lacking ambient capability support is detected any configured SystemCallFilter= and
 *      CapabilityBoundingSet= stanzas are implicitly modified, in order to permit spawned processes to drop
 *      credentials and capabilities themselves, even if this is configured to not be allowed. Moreover, if 
 *      this prefix is used and a system lacking ambient capability support is detected AmbientCapabilities=
 *      will be skipped and not be applied. On systems supporting ambient capabilities, "!!" has no effect
 *      and is redundant.
 *  
 *  "@", "-", and one of "+"/"!"/"!!" may be used together and they can appear in any order. 
 *  However, only one of "+", "!", "!!" may be used at a time. Note that these prefixes are also 
 *  supported for the other command line settings, i.e. ExecStartPre=, ExecStartPost=, ExecReload=,
 *  ExecStop= and ExecStopPost=. 
 */

class SystemDUnit {
public:

    // Constants used to create the output and error bitmasks
    static const unsigned OUTPUT_TO_JOURNAL = 0x01;
    static const unsigned OUTPUT_TO_CONSOLE = 0x02;
    static const unsigned OUTPUT_TO_FILE    = 0x04;

    enum ServiceType {
       SERVICE_TYPE_UNDEFINED,
       SERVICE_TYPE_SIMPLE,
       SERVICE_TYPE_FORKING,
       SERVICE_TYPE_ONESHOT, 
       SERVICE_TYPE_DBUS,
       SERVICE_TYPE_NOTIFY,
       SERVICE_TYPE_IDLE
    };

    SystemDUnit(char *name) {
            this->name = name;
            this->is_enabled        = false;
            this->remain_after_exit = false;
          
        };
    ~SystemDUnit() {  };

    boolean Start(boolean block) { return false;  };
    boolean Stop(boolean block) {  return false; };
    boolean Enable(boolean block) {  return false; };
    boolean Disable(boolean block) {  return false; };
    boolean IsEnabled() {  return false; };
    void AddDependency(SystemDUnit *dependency) { };
    
    vector<class SystemDUnit *> GetDependencies()  { };
    static class SystemDUnit *ParseSystemDServiceUnit(string servicename, fstream &fs);
 
private:
    boolean ParseUnitSection( fstream &fs);
    boolean ParseServiceSection( fstream &fs);
    boolean ParseInstallSection( fstream &fs);

    boolean StartServiceSimple(boolean blocking);
    boolean StartServiceForking(boolean blocking);
    boolean StartServiceOneShot(boolean blocking);
    boolean StartServiceDBus(boolean blocking);
    boolean StartServiceNotify(boolean blocking);
    boolean StartServiceIdle(boolean blocking);

    boolean StopServiceSimple(boolean blocking);
    boolean StopServiceForking(boolean blocking);
    boolean StopServiceOneShot(boolean blocking);
    boolean StopServiceDBus(boolean blocking);
    boolean StopServiceNotify(boolean blocking);
    boolean StopServiceIdle(boolean blocking);

    boolean ReloadServiceSimple(boolean blocking);
    boolean ReloadServiceForking(boolean blocking);
    boolean ReloadServiceOneShot(boolean blocking);
    boolean ReloadServiceDBus(boolean blocking);
    boolean ReloadServiceNotify(boolean blocking);
    boolean ReloadServiceIdle(boolean blocking);

    string name;
    boolean is_enabled;
    unsigned output_mask;
    unsigned error_mask;
    string   output_file_path;
    string   error_file_path;
    boolean remain_after_exit;
    boolean guess_main_pid;      // only interesting if type == forking
    string  pid_file;            // aboslute path of the pid file of the service. recommended for type = forking
    string  bus_name;            // d-bus bus name (in windows this is just a service id) . Required for type = dbus
    enum ServiceType service_type;

    double restart_sec;             // time to sleep before restarting a service. Default 100ms
    double timeout_start_sec;       // time to wait for startup. -1 is "infinity", Defaults to DefautTimeoutStopSec
    double timeout_stop_sec;
    double watchdog_sec;
    double max_runtime_sec;

    vector<string> exec_start_pre;  // Kept in order of definition
    vector<string> exec_start;      // Kept in order of definintion
    vector<string> exec_start_post; // Kept in order of definition
    vector<string> exec_reload;     // Kept in order of definintion
    vector<string> exec_stop;       // Kept in order of definintion
    vector<string> exec_stop_post;  // Kept in order of definition
    vector<class SystemDUnit *> dependencies;
};

class SystemDUnitPool {
public:
    SystemDUnitPool() {
               string system_drive;
               DWORD rslt = GetEnvironmentVariableA(
                    "SystemDrive", BUFFER, MAX_BUFFER_SIZE);
             
               if (rslt == 0) {
                   system_drive = "C:\\";
               }
               else {
                   system_drive = BUFFER;
               }
               
               UNIT_DIRECTORY_PATH = system_drive + "/SystemD/lib/";
               ACTIVE_UNIT_DIRECTORY_PATH = system_drive + "/SystemD/active/";
          };

    ~SystemDUnitPool() { };
 
    vector<class SystemDUnit *>GetPool() { return pool; };
    static class SystemDUnit *FindUnit(string name) {  return NULL; };
    static class SystemDUnit *ReadServiceUnit(string name) {
              string servicename = name;
              class SystemDUnit *punit = NULL;

              if (servicename.rfind(".service") == string::npos) {
                  servicename += ".service";
              }
              
              string service_unit_path = UNIT_DIRECTORY_PATH+servicename;
              // Find the unit in the unit directory
              fstream fs(service_unit_path, std::fstream::in);
			  
              if (fs.is_open()) {
                  string justname = servicename.substr(0, servicename.find_last_of('.'));
                  punit = SystemDUnit::ParseSystemDServiceUnit(justname, fs);
              }
              else {
                  cerr << "No service unit " << servicename.c_str() << "Found in unit library" << endl;
              }
              fs.close();

              fs.open(ACTIVE_UNIT_DIRECTORY_PATH+servicename);
              if (fs.is_open()) {
                  punit->Enable(true);
              }
 
              return punit;
         };

      // deserialises and loads the pool of objects. Throws an exception on failure

      void LoadPool() { };
      void SavePool() { };

private:
   static string UNIT_DIRECTORY_PATH;        // Quasi constant inited in contructor
   static string ACTIVE_UNIT_DIRECTORY_PATH; // Quasi constant
   vector<class SystemDUnit *> pool;
};

extern class SystemDUnitPool *g_pool;

#endif // __WIN_SYSTEMD_SERVICE_UNIT_H__
