# Lecture 1: Introduction and installation – how to install Geant4, QtCreator and ROOT

What Geant4 actually is – a C++ toolkit for writing particle-transport simulations, not a standalone program like TRIM, SIMION or LISE++ – and how to install everything used in this course: Geant4, QtCreator and ROOT. The installation is done on Linux; on Windows it is Linux inside WSL.

There is no course project in this lecture. Geant4's own basic example **B4a** is used to check that the installation works.

## Covered in the deck

- **What Geant4 is:** a large set of C++ classes you use in your own program; built-in physics processes with their cross sections; typical uses – detector response and efficiency, trajectories in fields, stopping ranges, energy losses. You can add your own processes.
- **Documentation:** the Geant4 documentation page, the Application Developer guide, the Physics List guide and the Physics Reference Manual.
- **WSL on Windows:** requirements (Windows 10 21H2+ or Windows 11, about 40 GB free on C:), `wsl --list --online`, `wsl --install -d Ubuntu-…`.
- **If WSL is not available:** native Linux or dual boot, a remote Linux machine over ssh, a virtual machine (VMware, VirtualBox), or – untested – a native Windows build with Visual Studio.
- **Updating Ubuntu:** `sudo`, `apt update` / `apt upgrade`, typing passwords in the terminal.
- **Dependencies for Geant4:** `cmake g++ gcc build-essential qt6-base-dev libqt6opengl6-dev qt6-base-dev-tools libxmu-dev libx11-dev git wget cmake-curses-gui libexpat1-dev libxerces-c-dev`.
- **Building Geant4 11.4.3 from source:**
  - separate `source`, `build` and `install` folders, so a new version is easy to install and relink;
  - `git clone --branch v11.4.3 --depth=1 https://github.com/Geant4/geant4.git`;
  - configuring with `ccmake`: `GEANT4_INSTALL_DATA=ON` (data sets), `GEANT4_USE_QT=ON` (visualization), `GEANT4_USE_GDML=ON`, `CMAKE_INSTALL_PREFIX=~/geant4/install`, and check that `GEANT4_BUILD_MULTITHREADED` is `ON`;
  - compiling and installing with `make -jN install` (N parallel compilations; it takes 20–40 minutes).
- **Geant4 environment and QtCreator:** adding `source ~/geant4/install/bin/geant4.sh` to `.bashrc`, installing QtCreator, opening `examples/basic/B4/B4a/CMakeLists.txt`, configuring, building and running it with the visualization window.
- **What `CMakeLists.txt` does:** CMake prepares the build system and `make` compiles; the file says where Geant4 is, which `.cc` files belong to the program, which executable to build, which libraries to link and which files to copy to the build folder.
- **Building ROOT from source** the same way: dependencies, `git clone --branch latest-stable`, `ccmake` with `http`, `xrootd` and `builtin-xrootd` switched off, install to `~/root/install`, `source ~/root/install/bin/thisroot.sh` in `.bashrc`.
- **When something breaks:** warnings usually do not matter; find the *first* real error in the log (not the last `make: ***` line), copy it with about 30 lines around it and ask your favourite LLM – this is much faster than reading the whole log. Errors in `ccmake` usually mean a missing dependency; errors in ROOT's `make install` usually mean `http` / `xrootd` were not switched off. Many "Terminated" messages without a clear error mean the build ran out of RAM – use fewer parallel jobs.
- **A cheat sheet of Linux terminal commands:** `pwd`, `ls`, `cd`, `mkdir`, `cp`, `mv`, `rm`, `clear`, `explorer.exe .` (open the current folder in Windows Explorer), TAB completion, command history, Ctrl+C, Ctrl+Shift+C / Ctrl+Shift+V.

## Expected result – "it all works" checklist

- `geant4-config --version` prints `11.4.3`.
- `geant4-config --check-datasets` shows the data sets as installed.
- QtCreator builds and runs B4a, the visualization window opens and shows the calorimeter layers, and tracking starts with the green "run" button.
- `root` opens a ROOT prompt.

The course applications write ROOT-format files through Geant4's analysis manager; ROOT is only used to look at them and is not needed to build the applications. 

