# Makefile for WasEngine
#
# Wraps MSBuild so the solution can be built, run, and cleaned from a
# terminal (PowerShell) without opening Visual Studio.
#
# Requires msbuild.exe to be on PATH. If you are not using a
# "Developer PowerShell for VS 2026" prompt, either:
#   - run these targets from one, or
#   - override MSBUILD below with the full path to MSBuild.exe, e.g.:
#       make build MSBUILD="C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe"

SOLUTION      := WasEngine.sln
CONFIGURATION ?= Debug
PLATFORM      ?= x64
MSBUILD       ?= msbuild.exe
EXE           := x64/$(CONFIGURATION)/WasEngine.exe

.PHONY: all build rebuild clean run stop

all: build

## Build the solution (default: Debug|x64)
build:
	$(MSBUILD) $(SOLUTION) /p:Configuration=$(CONFIGURATION) /p:Platform=$(PLATFORM) /m

## Force a full rebuild
rebuild:
	$(MSBUILD) $(SOLUTION) /t:Rebuild /p:Configuration=$(CONFIGURATION) /p:Platform=$(PLATFORM) /m

## Remove build outputs
clean:
	$(MSBUILD) $(SOLUTION) /t:Clean /p:Configuration=$(CONFIGURATION) /p:Platform=$(PLATFORM)

## Build (if needed) then launch the app
run: build
	powershell -NoProfile -Command "Start-Process -FilePath '$(EXE)'"

## Stop a running instance of the app
stop:
	powershell -NoProfile -Command "Stop-Process -Name WasEngine -Force -ErrorAction SilentlyContinue"
