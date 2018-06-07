# SystemCtl-win

Linux and Unix applications are often structured as a collection of of systemd service units, relying 
on the features of systemd to coordinate service sequencing restart, and dependencies.  The contents ofi
those service units are well defined by freedesktop.org, and amount to a scripting language based on bash.
The units are placed in a well known directories, defined by systemctl and executed by systemd.

In windows, services are specially written programs which have a sepcial mainline (ServiceMain) entry point, and 
dispatch control messages (start, stop, pause, continue) sent from the service control manager. The SCM is 
in turn controlled or queries by programs and UI elements such as sc.exe.  The SCM has a limited repetoire of 
controls to specify dependencies, working dirs, and iunder what user the service is running. An executable 
program is most definitely not a service though a service can often be configured to run as an executable with the 
right command line options.

The disconnect between these two approaches creeates a problem for anyone wishing to port a non trivial 
app from windows to linux.  Systemctl-win attempts to narrow the difference by
  - Providing a command line utility which duplicates, so far as possible, the command line syntax and function
    of the linux systemctl. Where there are clear differences, such as signals, a subset the linux syntax is
    retained even though the underlying mechanisms are quite different. So, 
    ```
    systemctl kill --signal SIGHUP myservice
    ```
    won't work and will get a message that SIGHUP is not supported, since Windows has no such concept, but 
    ```
    systemctl kill --signal SIGQUIT myservice
    ```
    will send a CTRL_BREAK  message to the target processes' console handler, which will achieve the desired effect
    (terminating the process).

  - Providing a service wrapper (systemd-exec) which reads and executes the service unit execution directives, 
    monitors execution and restarts as needed.

Design Goals:

  - As much as possible, make it convenient to port a systemd based application from linux to windows without requiring restructing..
  - Retain as much of the service unit content as possible given the differing semantics of the systems.
    
Design Non-Goals:
  - We do not believe it is possible or desirable to make it a goal to be able to accept linux service units
    verbatim in the windows environment. But it is possible to make it possible to use service unit files which are 
    immediately recognisable to functionally duplicate the equivielnt linux service.

### SystemD-Exec 

Among the major differences between windows and linux environments are:
  - shell environment: bash does not exist in a standard windows environment. cmd.exe and pwsh
   (powershell core) are present either by default or through standard package managers (like nuget).

  - logging:  Windows does not have a console in the linux sense. It also does not have syslog or journald
    which is very much involved in the routing of log text. Windows does have a very efficient EventLog mechanism
    which doesn't have a file-like interface.

  - signals: Windows does not have signals. Linux uses signals, usually SIGKILL, SIGHUP, SIGQUIT, SIGTERM, SIGUSR1 
    to control processes. Windows has the ability to send an event to a console process, in particular CTRL_C_EVENT and 
    CTRL_BREAK_EVENT. Only CRTL_C and CTRL_BREAK are documented. It will be up to the target application to 
    decide their effect.

  - file systems: Whereas linux is very permissive about sharing and removing files and directories, windows has
    severe limitations that must be respected.

#### SHELL extension

    Bash doesn't exist in a standard windows environment, systemd-exec extends the systemd unit file
    to allow the use of different shell environments.  In fact, cmd.exe is the native execution environment in 
    the windows enviornment, with powershell a standard alternative. But bash is available in several flavours
    such as msys2, cygwin, and the such, so it the service process is being executed as a user, this is also 
    possible to set the user path to include the path to the bash, cmd or powershell of your choice.
    ```
    SHELL=bash
    ```
    will make the system-exec embed the specified command for ExecStart, ExecStartPre, etc into a command for bash.
    So, 
    ```
    ExecStartPre=myservice --arg1 --arg2=$HOME
    ```
    becomes:
    ```
    bash -c myservice --arg1 --arg2=$HOME
    ```
    and 
    ```
    SHELL=pwsh
    ```








