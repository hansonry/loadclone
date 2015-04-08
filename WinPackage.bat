SET EXE7Z="C:\Program Files\7-Zip\7z.exe"
SET GAMENAME=loadclone


REM Copy create a release folder to copy all required files into
mkdir %GAMENAME%
copy *.dll %GAMENAME%
copy *.exe %GAMENAME%
copy *.png %GAMENAME%
copy testmap.txt %GAMENAME%
copy *.otf %GAMENAME%
copy how_to_play.md %GAMENAME%

REM Create a zip file with all the required files
%EXE7Z% a -tzip %GAMENAME%.zip %GAMENAME%\

REM Delete Game Folder
rd /s /q %GAMENAME%

