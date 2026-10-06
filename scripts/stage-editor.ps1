#Requires -Version 5.1
<#
    Builds RevokeCraft in the Dist configuration and assembles dist/MyRevoke - a
    folder that runs the editor on a machine with no Visual Studio and no repo
    checkout.

    Target machines still need the Microsoft Visual C++ 2015-2022 Redistributable
    (x64): premake5.lua keeps staticruntime "off", so the MSVC runtime is not
    linked into the exe.
#>
[CmdletBinding()]
param(
    [string] $PremakeAction = 'vs2022',
    [string] $OutputPath,
    [switch] $SkipBuild
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$editorDir = Join-Path $repoRoot 'RevokeCraft'
if (-not $OutputPath)
{
    $OutputPath = Join-Path (Join-Path $repoRoot 'dist') 'MyRevoke'
}

# Import libraries, debug info and the patched copies NativeScript.cpp writes
# beside the script DLL at runtime all sit in the same folders as the files the
# editor loads, so they are pruned after the copy instead of filtered during it.
$leftoverExtensions = '.lib', '.exp', '.pdb', '.ilk'
$leftoverNames = 'imgui.ini', '*_.dll', '*_dll', '*_pdb'

function Invoke-DistBuild
{
    $premake = Join-Path $repoRoot 'vendor\premake\premake5.exe'
    & $premake $PremakeAction
    if ($LASTEXITCODE -ne 0)
    {
        throw "premake5 $PremakeAction failed."
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild)
    {
        throw 'MSBuild not found. Install Visual Studio or the Visual Studio Build Tools.'
    }

    & $msbuild (Join-Path $repoRoot 'MyRevoke.sln') -m -nologo '-t:RevokeCraft' '-p:Configuration=Dist' '-p:Platform=x64'
    if ($LASTEXITCODE -ne 0)
    {
        throw 'Dist build of RevokeCraft failed.'
    }
}

function Remove-BuildLeftovers($root)
{
    foreach ($directory in @(Get-ChildItem $root -Recurse -Directory -Filter 'intermediates'))
    {
        Remove-Item $directory.FullName -Recurse -Force
    }

    foreach ($file in @(Get-ChildItem $root -Recurse -File))
    {
        $isLeftover = $leftoverExtensions -contains $file.Extension
        foreach ($pattern in $leftoverNames)
        {
            if ($file.Name -like $pattern)
            {
                $isLeftover = $true
            }
        }

        if ($isLeftover)
        {
            Remove-Item $file.FullName -Force
        }
    }
}

if (-not $SkipBuild)
{
    Invoke-DistBuild
}

$exe = Join-Path $editorDir 'RevokeCraft.exe'
if (-not (Test-Path $exe))
{
    throw "RevokeCraft.exe not found at $exe."
}

if (Test-Path $OutputPath)
{
    Remove-Item $OutputPath -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null

# The editor opens its resources relative to its own exe (GetExecutableDirectory),
# so the staged layout has to match the folder RevokeCraft builds into.
Copy-Item $exe -Destination $OutputPath
Copy-Item (Join-Path $editorDir 'OpenAL32.dll') -Destination $OutputPath
Copy-Item (Join-Path $editorDir 'resourses') -Destination $OutputPath -Recurse
Copy-Item (Join-Path $editorDir 'projects') -Destination $OutputPath -Recurse

# Only lib/ of the Mono distribution; the embedded runtime reads its class
# libraries from mono/lib and nothing else in mono/ is used at runtime.
$monoDestination = Join-Path $OutputPath 'mono'
New-Item -ItemType Directory -Path $monoDestination -Force | Out-Null
Copy-Item (Join-Path (Join-Path $editorDir 'mono') 'lib') -Destination $monoDestination -Recurse

Remove-BuildLeftovers $OutputPath

$stagedFiles = @(Get-ChildItem $OutputPath -Recurse -File)
$totalBytes = ($stagedFiles | Measure-Object -Property Length -Sum).Sum
Write-Host ('Staged {0} files, {1:N1} MB, in {2}' -f $stagedFiles.Count, ($totalBytes / 1MB), $OutputPath)
Write-Host 'Target machines need the Microsoft Visual C++ 2015-2022 Redistributable (x64).'
