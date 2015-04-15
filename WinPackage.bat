SET EXE7Z="C:\Program Files\7-Zip\7z.exe"
SET GAMENAME=loadclone
SET TEMPDIR=tmp
SET OUTPUTDIR=release
SET DEST=%TEMPDIR%\%GAMENAME%


REM Copy create a release folder to copy all required files into
mkdir %TEMPDIR%
mkdir %DEST%

copy *.dll          %DEST%
copy *.exe          %DEST%
copy *.png          %DEST%
copy *_levelset.txt %DEST%
copy *_map.txt      %DEST%
copy config.txt     %DEST%
copy *.otf          %DEST%
copy how_to_play.md %DEST%

REM Create a zip file with all the required files
cd %TEMPDIR%
%EXE7Z% a -tzip ..\%OUTPUTDIR%\%GAMENAME%.zip %GAMENAME%\
cd ..

REM Delete Game Folder
rd /s /q %TEMPDIR%

