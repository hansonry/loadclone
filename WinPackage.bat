SET EXE7Z="C:\Program Files\7-Zip\7z.exe"
SET GAMENAME=loadclone
SET TEMPDIR=tmp
SET OUTPUTDIR=release
SET DEST=%TEMPDIR%\%GAMENAME%
SET OUTPUT_ZIP=..\%OUTPUTDIR%\%GAMENAME%.zip


REM Rebuild Project
bam -c
bam

REM Copy create a release folder to copy all required files into
mkdir %TEMPDIR%
mkdir %DEST%


copy *.dll               %DEST%
copy *.exe               %DEST%
copy *.png               %DEST%
copy *_levelset.txt      %DEST%
copy *_map.txt           %DEST%
copy config_template.txt %DEST%
copy *.otf               %DEST%
copy how_to_play.md      %DEST%
copy *.wav               %DEST%
copy *.ogg               %DEST%

REM Create a zip file with all the required files
cd %TEMPDIR%

del %OUTPUT_ZIP%
%EXE7Z% a -tzip %OUTPUT_ZIP% %GAMENAME%\
cd ..

REM Delete Game Folder
rd /s /q %TEMPDIR%

