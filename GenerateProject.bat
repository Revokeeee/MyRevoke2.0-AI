@echo off
rem Usage: GenerateProject.bat [action]   e.g. vs2026 (default) or vs2022
set ACTION=%1
if "%ACTION%"=="" set ACTION=vs2026
call vendor\premake\premake5.exe %ACTION%
PAUSE
