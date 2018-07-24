#
# regress.ps1 $bindir

 [CmdletBinding()]
 param (
     [Parameter(Position=0)]
     [string]$bindir,
     [Parameter(Position=1)]
     [string]$logfile = ""
 )

function Write-Log () { 
  param (
     [Parameter(Position=0)]
     [string]$msg,
     [Parameter(Position=0)]
     [string]$color = "White"
 )
    if($host.UI.RawUI.ForegroundColor -ne $null) {
        Write-Host "`n[$([datetime]::Now.ToLongTimeString())] $msg" -ForegroundColor $color
    } else {
        Write-Output "`r`n[$([datetime]::Now.ToLongTimeString())] $msg" 
    }
}

function TestSuccess($msg) {
    $msg = ("==== Test Success: "+$msg)
    if($host.UI.RawUI.ForegroundColor -ne $null) {
       Write-Host "`n[$([datetime]::Now.ToLongTimeString())] $msg" -ForegroundColor "Green"
    } else {
       $host.UI.WriteErrorLine("`r`n[$([datetime]::Now.ToLongTimeString())] $msg" )
    }
}

function TestFailed($msg) {
    if($host.UI.RawUI.ForegroundColor -ne $null) {
       Write-Host "`n[$([datetime]::Now.ToLongTimeString())] $msg" -ForegroundColor "Red"
    } else {
       $host.UI.WriteErrorLine("`r`n[$([datetime]::Now.ToLongTimeString())] $msg" )
    }
    $Error.clear()
}

$test_service_dir="$PWD/service_units"
$servicedir="c:/etc/systemd/system"
$activedir ="c:/etc/systemd/active"
$logdir="c:/var/log"

rm "$activedir/*"

if (!$bindir) {
    $bindir="$PSScriptRoot/../x64/Release"
}

#
#  Enable service a also enables b, f
#
try {
    copy-item -recurse -path "$test_service_dir/*" -destination $servicedir
    $Error.clear()
    $results = invoke-expression " $bindir/systemctl.exe enable a.service" 
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service A failed"
    }
    else {
         $check = (get-service | where { $_.name -eq "a.service" })
         if ($check.count -eq 0) {
            throw "enable Service A failed to enable service"
         }
         if ($check.count -ne 1) {
             throw "enable Service A enabled extra service instances"
         }
         $check = (get-service | where { $_.name -eq "b.service" })
         if ($check.count -eq 0) {
            throw "enable Service A failed to enable dependent service b"
         }
         if ($check.count -ne 1) {
             throw "enable Service A enabled extra service b instances"
         }
         $check = (get-service | where { $_.name -eq "f.service" })
         if ($check.count -eq 0) {
            throw "enable Service A failed to enable service b dependent service f"
         }
         if ($check.count -ne 1) {
             throw "enable Service A enabled extra service f instances"
         }
         TestSuccess "Test EnableServiceA Success"
    } 
}
catch {
   TestFailed "EnabledServiceA:$Error"
}


try {   
    rm "$logdir/test-a*"
    invoke-expression "$bindir/systemctl.exe start a.service"
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service A failed"
    }
    else {
         TestSuccess "Test StarterviceA Success"
    } 
    $check = (get-service | where { $_.name -eq "a.service" })
}
catch {
   TestFailed "Test StartServiceA failed: $Error"
}

#
# Enable With Multiple Dependents

try {
    invoke-expression " $bindir/systemctl.exe enable i.service"
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service i failed"
    }
    else {
         $check = (get-service | where { $_.name -eq "i.service" })
         if ($check.count -eq 0) {
            throw "enable Service with multiple dependents i failed to enable service"
         }
         if ($check.count -ne 1) {
             throw "enable Service with multiple dependents i enabled extra service instances"
         }
         $check = (get-service | where { $_.name -eq "b.service" })
         if ($check.count -eq 0) {
            throw "enable Service with multiple dependents i failed to enable dependent service b"
         }
         if ($check.count -ne 1) {
             throw "enable Service with multple dependents A enabled extra service b instances"
         }
         $check = (get-service | where { $_.name -eq "f.service" })
         if ($check.count -eq 0) {
            throw "enable Service with multiple dependents I failed to enable service b dependent service f"
         }
         if ($check.count -ne 1) {
             throw "enable Service with multiple dependents I enabled extra service f instances"
         }
         TestSuccess "Test EnableServiceWithMultipleDependents Success"
    } 
}
catch {
   TestFailed "Test EnableServiceI failed: $Error"
}


#
# Enable running service
#

try {
   invoke-expression " $bindir/systemctl.exe enable running.service"
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service running failed"
    }
    else {
         $check = (get-service | where { $_.name -eq "running.service" })
         if ($check.count -eq 0) {
            throw "enable running service failed to enable service"
         }
         if ($check.count -ne 1) {
            throw "enable running service enabled extra service instances"
         }
         invoke-expression " $bindir/systemctl.exe start running.service"

         $check.WaitForStatus("Running", "00:05:00")

         if ($check.Status -ne "Running") {
             throw "start running service timedout"
         }

         invoke-expression " $bindir/systemctl.exe stop running.service"
         $check.WaitForStatus("Stopped", "00:05:00")

         if ($check.Status -ne "Stopped") {
             throw "stop running service timedout"
         }

         invoke-expression " $bindir/systemctl.exe mask running.service"

         $check = (get-service | where { $_.name -eq "running.service" })
          if ($check.count -ne 0) {
            throw "mask running service did not clear service"
         }
         if (Test-Path -Path "$activedir/running.service") {
            throw "mask running service did not clear service unit"
         }

         TestSuccess "Test EnableStartStopRunningService Success"
    } 
}
catch {
   TestFailed "Test EnableStartStopRunningService failed: $Error"
}
