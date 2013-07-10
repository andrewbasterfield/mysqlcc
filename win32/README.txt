COMPILE INSTRUCTIONS FOR WIN32 & Visual C++

Copy mysqlcc.pro to the root directory.  From the command prompt type:

qmake -t vcapp -o mysqlcc.dsp mysqlcc.pro

That will generate a file called mysqlcc.dsp which can be opened by Visual C++