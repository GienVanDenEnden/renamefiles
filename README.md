renamefiles
============

Quick and easy rename files in 1 directory

Description
-----------

Rename all the selected files too the given rename pattern.
The filename is divided into parts with the point (.) as seperator.
There is only 1 wildchar, a star (*) witch selected 0, 1 of more characters.
A wildchar belongs too a filename part.
For each part, the wildchars in the rename pattern correspontent with the wildchars
in the file selection. Speciale case: if the file-selection contains no wildchar and
the rename part only contains a wildchar then copy the filename part

Installation
------------
```
OS             Linux
Compile        make
Installation   sudo make install
Remove         sudo make uninstall
```

Options
-------
* `--help`      show help and exit
* `--version`   show version and exit
* `--norename`  show only filenames too rename
* `--noheader`  display no header information
* `--nofooter`  display no footer information
* `--nofilenames` display no filenames
* `--nooutput` display no information only errors
* `--nostarcheck` no check for a wildchar (*) in the file selection
* `--failmatchinfo` display filenames witch do not match and the reason
* `--nonamesplit` use the filename as 1 string

Examples
--------
```
renamefiles "*.c" "*.cc"
   Rename all the files with the extension c too cc
   
renamefiles "*_test.c" "*.c"
   Remove all the _test and the end of a filename, file_test.c -> test.c

renamefiles "*.part*.*" "*.*.*"
   Remove the part from the second part of the filename, test.part01.rar -> test.01.rar
   
renamefiles "*" "*.c"
   Give all the filenames with no extension the .c extensie
   
renamefiles "*.c" "prog_*.c"
   Let all the filenames with the .c extension begin with prog_, test.c -> prog_test.c
   
renamefiles "*name*.o" "**.*"
   Remove the name from the filename, renamefiles.o -> refiles.o
   
renamefiles /home/user/test "*.c" "*.cc"
   Rename all the files in the /home/user/test directory with the extension c too cc
   
renamefiles "*_test.c" "*.*"
   Speciale case: file_test.c -> file.c
   
renamefiles -nonamesplit "*test*" "*"
   Remove test from filename: filetest1.c -> file1.c
```

License
-------
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

