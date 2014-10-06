/*uudoall.c cleans up split files checks checksums and writes by h**2 7/4/91
 *  (c) 1991 by Howard Helman
 */



/*
  uudoall.c can usually handle the split files with all the intervening mail
  crap.
  Automatically checks for checksums on lines and automatically checks for
  a `size' line at the end and verifies the size.  It also computes the sum
  value for the decoded file. I could not find any convention on people using
  sum so the program just computes it and prints it.

  Compile by defining DOS for msdos use or OS2 for GCC on OS/2 or UNIX for use elsewhere.
 */


/* the program is called by :
        uudoall [opts] -ddir infile [...]
 where opts are:
                -s+             do size check
                -s-             inhibit size check
                -c+             do line by line checksum
                -c-             inhibit checksum
                -ddir   place output files in the named directory
                infile  the file to process (all parts should be concatenated.)
                [...]   options and file names may be repeated.
                                The last option setting is in effect when a file is
                                processed.
 If the program cannot open the output file, it prompts the user
 for a new file name or `enter' to stop.
 The trimming of trailing blanks is handled properly.  This program
 has been in use continually and has only had one problem: a user
 signature started in col 1 and had a `M' and over sixty characters in the
 line.  This is what the program searches for to find the next part of the
 encoded file.
*/

/*
  This program is donated to the public domain provided that the source
  is distributed in total and my authorship of the program is credited.

                Howard Helman
                Box 340
                Manhattan Beach
                CA 90266
*/

/* v1.1, 3 Oct 91   Toad Hall Tweak
 * - Slight tweaks to reduce Turbo C v2.0 warnings.
 * - Threw in some prototypes.
 * - Reformatted to K&R standards (via good old Mister Mangler,
 *   AKA indent).
 * David Kirschbaum
 * Toad Hall
 * kirsch@usasoc.soc.mil
 */

/* Added #ifdef's for OS/2 August 29, 1993 */

#ifdef __TURBOC__
#define DOS 1
#endif

#ifdef DOS
#include <stdlib.h>
#include <alloc.h>
#define ReaD "rt"
#define WritE "wb"
#define SlasH "\\"
#endif
#ifdef UNIX
#define ReaD "r"
#define WritE "w"
#define SlasH "/"
#endif
#ifdef OS2
#define ReaD "rt"
#define WritE "wb"
#define SlasH "/"
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define Bsize 80
#define SUMSIZE 64
#define DEC(c)  (((x_x=(c))>=' ')?((x_x-' ')&077):(0))  /* single char decode */
#define match(x) matcher(buf,x)

static int dosize = 1, docheck = 1;
static char x_x, dir[80];
static long nbytes;
static unsigned int sum1;

#ifdef DOS
static void *vbi;
static unsigned bn;
#endif

#ifdef __TURBOC__
/* v1.1 I like prototypes */
int matcher(char *a, char *b);
int hget(char *buf, int bs, FILE *in);
void decode(FILE *in, FILE *out, char *dest);
void doopts(char *s);
void decodeit(FILE *in, char buf[Bsize]);

#endif
enum States {
        BeginSearch, InBody, GapOrEnd, LookEnd, PutTwo, PutEnd,
        Mfind
} state = BeginSearch;

int matcher(a, b)
char *a, *b;
{
        while (*b && *a == *b)
                a++, b++;
        return (!*b);
}

int hget(buf, bs, in)
char *buf;
int bs;
FILE *in;
{
        static char s1[Bsize], s2[Bsize];

        if (state == PutEnd) {
                strcpy(buf, "end");
                state = BeginSearch;
                return 1;
        }
        if (state == PutTwo) {
                strcpy(buf, s2);
                state = PutEnd;
                return 1;
        }
        while (fgets(buf, bs, in)) {
                switch (state) {
                case BeginSearch:
                        if (match("begin "))
                                state = InBody;
                        return 1;
                case InBody:
                        if (*buf == 'M')
                                return 1;
                        else if (*buf == '`') {
                                state = BeginSearch;
                                return 1;
                        } else {
                                strcpy(s1, buf);
                                state = GapOrEnd;
                        }
                        break;
                case GapOrEnd:
                        if (match("end")) {
                                strcpy(buf, s1);
                                state = PutEnd;
                                return 1;
                        } else {
                                strcpy(s2, buf);
                                state = LookEnd;
                        }
                        break;
                case LookEnd:
                        if (match("end")) {
                                strcpy(buf, s1);
                                state = PutTwo;
                                return 1;
                        } else
                                state = Mfind;
                        break;
                case Mfind:
                        if (*buf == 'M' && strlen(buf) > 60) {
                                state = InBody;
                                return 1;
                        }
                        break;
                }
        }
        return 0;
}

void decode(in, out, dest)
FILE *in, *out;
char *dest;
{
        int j, n, checksum, altsum;
        char buf[Bsize], *bp, hold[Bsize / 4 * 3], *jp;
        unsigned pcline = 0, line = 0;

        sum1 = 0;
        nbytes = 0;
        while (memset(buf, 0, Bsize), hget(buf, Bsize, in)
          &&(n = DEC(*buf)) > 0) {
                line++;
                altsum = checksum = 0;
                bp = buf + 1;
                for (jp = hold, j = n; j > 0; j -= 3, bp += 4) {
                        *jp = (DEC(bp[0]) << 2) | (DEC(bp[1]) >> 4);
                        checksum += *jp++;
                        *jp = (DEC(bp[1]) << 4) | (DEC(bp[2]) >> 2);
                        checksum += *jp++;
                        *jp = (DEC(bp[2]) << 6) | (DEC(bp[3]));
                        checksum += *jp++;
                        altsum += bp[0] + bp[1] + bp[2] + bp[3];
                }
                for (j = 0, jp = hold; j < n; j++, jp++)
                        sum1 = ((sum1 >> 1) + ((sum1 & 1) ? 0x8000 : 0)
                                + (*jp & 0xff)) & 0xffff;
                nbytes += n;
                if (fwrite(hold, 1, n, out) != n) {
                        fprintf(stderr, "ERROR: error writing to %s\n", dest);
                        exit(1);
                }
                if (!isspace(*bp) && docheck)
                        if ((checksum & (SUMSIZE - 1)) != DEC(*bp)
                          && (altsum & 077) != (DEC(*bp) & 077)) {
                                if (!pcline)
                                        pcline = line;
                        } else if (pcline) {
                                fprintf(stderr,
                                                "ERROR:Bad Checksum lines %u-%u\n", pcline, line);
                                pcline = 0;
                        }
        }
        if (pcline)
                fprintf(stderr, "ERROR:Bad Checksum lines %u-%u\n", pcline, line);
        if (feof(in) ||ferror(in)) {
                fprintf(stderr, "ERROR: Input ended unexpectedly!\n");
                exit(1);
        }
}

void decodeit(in, buf)
FILE *in;
char buf[Bsize];
{
        int mode;
        long filesize, tsize;
        FILE *out;
        char fname[128], dest[128];

        sscanf(buf, "begin %o %s", &mode, dest);
        while (strcpy(fname, dir) && strcat(fname, dest)
          && !(out = fopen(fname, WritE))) {
                fprintf(stderr, "ERROR: Can't open output file %s\nTry new one:",
                                fname);
                gets(dest);
                if (!*dest)
                        exit(1);
        }
#ifdef DOS
        setvbuf(out, (char *) vbi + bn * 512, _IOFBF, bn * 512);
#endif
        fprintf(stderr, "Creating %s\n", fname);
        decode(in, out, dest);
        if (!hget(buf, Bsize, in) ||!match("end")) {
                fprintf(stderr, "ERROR: no `end' line\n");
                exit(1);
        }
        tsize = ftell(out);
        if (dosize && hget(buf, Bsize, in) &&match("size ")) {
                sscanf(buf, "size %ld", &filesize);
                if (tsize != filesize) {
                        fprintf(stderr,
                        "ERROR: file should have been %ld bytes long but was %ld.\n",
                                        filesize, tsize);
                        exit(1);
                }
        } else if (dosize)
                fprintf(stderr, "Size check not done\n");
        if (tsize != nbytes) {
                fprintf(stderr, "Size Error:file=%ld data=%ld\n", tsize, nbytes);
                exit(1);
        }
        fprintf(stderr, "sums: %u %ld %ld\n", sum1, (nbytes + 1023) / 1024,
                        nbytes);
        fclose(out);
}

void doopts(s)
char *s;
{
        if (s[1] == 's')
                dosize = s[2] != '-';
        else if (s[1] == 'c')
                docheck = s[2] != '-';
        else if (s[1] == 'd') {
                strcpy(dir, s + 2);
                if (*dir)
                        strcat(dir, SlasH);
        } else {
                fprintf(stderr, "Illegal flag %s\n", s);
                exit(1);
        }
}

int main(argc, argv)
int argc;
char **argv;
{
        FILE *in;
        int nf = 0, i;
        char buf[80];

        *dir = 0;
        if (argc < 2) {
                fprintf(stderr,
        "UUDOALL is a UUDECODER for multiple part uuencoded files that have been placed\n");
                fprintf(stderr,
        "in a single file. For example, all of the articles from a USENET newsgroup\n");
                fprintf(stderr,
        "could be saved to a single file.  UUDOALL would automatically extract any\n");
                fprintf(stderr,
        "complete UUENCODED files.  It will handle UNIX and OS/2 long filenames.\n\n");
                fprintf(stderr,
        "USAGE: uudoall [opts] -ddir infile [...] where opts are -s+|- -c+|-\n");
                exit(1);
        }
#ifdef DOS
        bn = coreleft() / 1024 - 2;
        vbi = malloc(512 * 2 * bn);
#endif
        for (i = 1; i < argc; i++) {
                if (argv[i][0] == '-')
                        doopts(argv[i]);
                else if (!(in = fopen(argv[i], ReaD)))
                        fprintf(stderr, "ERROR: can't find %s\n", argv[i]);
                else {
#ifdef DOS
                        setvbuf(in, vbi, _IOFBF, bn * 512);
#endif
                        while (hget(buf, Bsize, in)) {
                                if (match("begin ")) {
                                        nf++;
                                        decodeit(in, buf);
                                }
                        }
                        fclose(in);
                }
        }
        return !nf;
}
