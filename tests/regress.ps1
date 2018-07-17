#
# regress.ps1 $bindir

 [CmdletBinding()]
 param (
     [Parameter(Position=0)]
     [string]$bindir
 )

$test_service_dir="$PWD/service_units"
$servicedir="c:/etc/systemd/system"
$activedir ="c:/etc/systemd/active"
$logdir="c:/var/log"

if (!$bindir) {
    $bindir="$PSScriptRoot/../x64/Release"
}


#
#  Enable service a also enables b, f
#
try {
    copy-item -recurse -path "$test_service_dir/*" -destination $servicedir
    $Error.clear()
    invoke-expression " $bindir/systemctl.exe enable a.service"
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
         Write-Host "Test EnableServiceA Success"
    } 
}
catch {
   Write-Host "Test EnableServiceA failed: $Error"
}


try {   
    rm "$logdir/test-a*"
    invoke-expression "$bindir/systemctl.exe start a.service"
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service A failed"
    }
    else {
         Write-Host "Test StarterviceA Success"
    } 
    $check = (get-service | where { $_.name -eq "a.service" })
}
catch {
   Write-Host "Test StartServiceA failed: $Error"
}


try {
    invoke-expression " $bindir/systemctl.exe enable i.service"
    if ($LASTEXITCODE -ne 0) {
        throw "enable Service i failed"
    }
    else {
         $check = (get-service | where { $_.name -eq "i.service" })
         if ($check.count -eq 0) {
            throw "enable Service i failed to enable service"
         }
         if ($check.count -ne 1) {
             throw "enable Service i enabled extra service instances"
         }
         $check = (get-service | where { $_.name -eq "b.service" })
         if ($check.count -eq 0) {
            throw "enable Service i failed to enable dependent service b"
         }
         if ($check.count -ne 1) {
             throw "enable Service A enabled extra service b instances"
         }
         $check = (get-service | where { $_.name -eq "f.service" })
         if ($check.count -eq 0) {
            throw "enable Service I failed to enable service b dependent service f"
         }
         if ($check.count -ne 1) {
             throw "enable Service I enabled extra service f instances"
         }
         Write-Host "Test EnableServiceI Success"
    } 
}
catch {
   Write-Host "Test EnableServiceI failed: $Error"
}


