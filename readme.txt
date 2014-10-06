This is a port of the DOS/Unix UUDOALL UUDECODER to OS/2.

This is the first 32-bit UUDECODER that I've seen ported that will decode
messages that include extraneous lines such as mail headers, and supports HPFS
filenames.

UUDOALL can decode UUENCODED files that have been sent via email in multiple
parts - provided that the files are all copied into one file before running
UUDOALL.

For example, if the OS/2 readme file were UUENCODED into 5 parts README1.UUE,
README2.UUE, README3.UUE, README4.UUE, and README5.UUE, the following command
would UUDECODE it:

copy README?.UUE ALL.UUE & uudoall ALL.UUE

UUDOALL was compiled with EMX 0.8f and requires the EMX*.DLL files to be
available in your OS/2 libpath.

(UUDOALL is especially convenient for using with the NNGRAB program, since it
can decode all of the binaries in a newsgroup such as COMP.BINARIES.OS2 in one
pass.)

Enjoy!

Albert Crosby
acrosby@uafhp.uark.edu

