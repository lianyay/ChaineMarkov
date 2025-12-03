@echo off
echo Construction du Projet TI301 Markov...
if exist build rmdir /s /q build
mkdir build
cd build
cmake ..
make
echo === Execution du Programme ===
TI_301_PJT.exe
pause
