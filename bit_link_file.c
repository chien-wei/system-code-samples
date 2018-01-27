#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h> 
#include <unistd.h>


/*********************************************************************
 *
 * These C functions use patterns and functionality often found in
 * operating system code. Your job is to implement them. Of course you
 * should write test cases. However, do not hand in your test cases
 * and (especially) do not hand in a main() function since it will
 * interfere with our tester.
 *
 * Additional requirements on all functions you write:
 *
 * - you may not refer to any global variables
 *
 * - you may not call any functions except those specifically
 *   permitted in the specification
 *
 * - your code must compile successfully on CADE lab Linux
 *   machines when using:
 *
 * /usr/bin/gcc -O2 -fmessage-length=0 -pedantic-errors -std=c99 -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter -Wshadow -Wuninitialized -Wold-style-definition -c assign1.c 
 *
 * NOTE 1: Some of the specifications below are specific to 64-bit
 * machines, such as those found in the CADE lab.  If you choose to
 * develop on 32-bit machines, some quantities (the size of an
 * unsigned long and the size of a pointer) will change. Since we will
 * be grading on 64-bit machines, you must make sure your code works
 * there.
 *
 * NOTE 2: You should not need to include any additional header files,
 * but you may do so if you feel that it helps.
 *
 * HANDIN: submit your finished file, still called assign1.c, in Canvas
 *
 *
 *********************************************************************/

/*********************************************************************
 *
 * byte_sort()
 *
 * specification: byte_sort() treats its argument as a sequence of
 * 8 bytes, and returns a new unsigned long integer containing the
 * same bytes, sorted numerically, with the smaller-valued bytes in
 * the lower-order byte positions of the return value
 * 
 * EXAMPLE: byte_sort (0x0403deadbeef0201) returns 0xefdebead04030201
 *
 *********************************************************************/

unsigned long byte_sort(unsigned long arg) {
    unsigned long result = 0;
    uint8_t nums[8];
    //printf("%d %d\n", sizeof(result), sizeof(arg));

    for (int i = 0; i < 8; i++) {
        // get the last bytes to var num, then put it into array nums
        uint8_t num = arg & 0xff;
        arg = arg >> 8;
        nums[i] = num;
    }

    // sort nums array, I use bubble sort here
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < (8 - 1); j++) {
            if (nums[j] < nums[j + 1]) {
                int tmp = nums[j + 1];
                nums[j + 1] = nums[j];
                nums[j] = tmp;
            }
        }
    }

    // join nums
    for (int i = 0; i < 8; i++) {
        if (i != 0) result = result << 8;
        result += nums[i];
    }
    //printf("%l016x\n", result);
    return result;
}

/*********************************************************************
 *
 * nibble_sort()
 *
 * specification: nibble_sort() treats its argument as a sequence of 16 4-bit
 * numbers, and returns a new unsigned long integer containing the same nibbles,
 * sorted numerically, with smaller-valued nibbles towards the "small end" of
 * the unsigned long value that you return
 *
 * the fact that nibbles and hex digits correspond should make it easy to
 * verify that your code is working correctly
 * 
 * EXAMPLE: nibble_sort (0x0403deadbeef0201) returns 0xfeeeddba43210000
 *
 *********************************************************************/

unsigned long nibble_sort(unsigned long arg) {
    unsigned long result = 0;
    uint8_t nums[16];

    for (int i = 0; i < 16; i++) {
        // get the last 4-bit to var num, then put it into array nums
        uint8_t num = arg & 0xf;
        arg = arg >> 4;
        nums[i] = num;
    }

    // sort nums array, I use bubble sort here
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < (16 - 1); j++) {
            if (nums[j] < nums[j + 1]) {
                int tmp = nums[j + 1];
                nums[j + 1] = nums[j];
                nums[j] = tmp;
            }
        }
    }

    // join nums
    for (int i = 0; i < 16; i++) {
        if (i != 0) result = result << 4;
        result += nums[i];
    }
    return result;
}

/*********************************************************************
 *
 * name_list()
 *
 * specification: allocate and return a pointer to a linked list of
 * struct elts
 *
 * - the first element in the list should contain in its "val" field the first
 *   letter of your first name; the second element the second letter, etc.;
 *
 * - the last element of the linked list should contain in its "val" field
 *   the last letter of your first name and its "link" field should be a null
 *   pointer
 *
 * - each element must be dynamically allocated using a malloc() call
 *
 * - if any call to malloc() fails, your function must return NULL and must also
 *   free any heap memory that has been allocated so far; that is, it must not
 *   leak memory when allocation fails
 *  
 *********************************************************************/

struct elt {
    char val;
    struct elt *link;
};

struct elt *name_list(void) {
    const char name[10] = "Chien-Wei";
    struct elt *head = (struct elt*) malloc(sizeof (struct elt));
    struct elt *cur = NULL;
    struct elt *tmp = NULL;

    if (!head) return NULL;
    head->val = name[0];
    cur = head;

    for (int i = 1; i < 10; i++) {
        cur -> link = (struct elt*) malloc(sizeof (struct elt));

        // handle failures
        if (!cur -> link) {
            // handle previous memory allocation
            while (head) {
                tmp = head -> link;
                free(head);
                head = tmp;
            }
            return NULL;
        }

        // malloc succeed, then assign value to its link
        cur -> link -> val = name[i];
        cur = cur->link;
    }

    return head;
}

/*********************************************************************
 *
 * convert()
 *
 * specification: depending on the value of "mode", print "value" as
 * octal, binary, or hexidecimal to stdout, followed by a newline
 * character
 *
 * extra requirement 1: output must be via putc()
 *
 * extra requirement 2: print nothing if "mode" is not one of OCT,
 * BIN, or HEX
 *
 * extra requirement 3: all leading/trailing zeros should be printed
 *
 * EXAMPLE: convert (HEX, 0xdeadbeef) should print
 * "00000000deadbeef\n" (including the newline character but not
 * including the quotes)
 *
 *********************************************************************/

enum format_t {
    OCT = 66, BIN, HEX
};

static char get_hex_value(int i) {
    if (i == 15) return 'f';
    else if (i == 14) return 'e';
    else if (i == 13) return 'd';
    else if (i == 12) return 'c';
    else if (i == 11) return 'b';
    else if (i == 10) return 'a';
    else if (i < 10 && i >= 0) return i + '0';
    return ' ';
}

static char get_oct_value(int i) {
    if (i < 8 && i >= 0) return i + '0';
    return ' ';
}

void convert(enum format_t mode, unsigned long value) {
    switch (mode) {
        case OCT:
            for (int i = 0; i < 22; i++) {
                int c = (value >> (21 - i) * 3) & 0x7;
                c = get_oct_value(c);
                if (c != ' ') putc(c, stdout);
            }
            putc('\n', stdout);
            break;

        case BIN:
            for (int i = 0; i < 64; i++) {
                int c = (value >> (63 - i)) & 1;
                c = c + '0';
                putc(c, stdout);
            }
            putc('\n', stdout);
            break;

        case HEX:
            for (int i = 0; i < 16; i++) {
                int c = (value >> (15 - i) * 4) & 0xf;
                c = get_hex_value(c);
                if (c != ' ') putc(c, stdout);
            }
            putc('\n', stdout);
            break;
    }
    return;
}

/*********************************************************************
 *
 * draw_me()
 *
 * this function creates a file called me.txt which contains an ASCII-art
 * picture of you (it does not need to be very big). the file must (pointlessly,
 * since it does not contain executable content) be marked as executable.
 * 
 * extra requirement 1: you may only call the function syscall() (type "man
 * syscall" to see what this does)
 *
 * extra requirement 2: you must ensure that every system call succeeds; if any
 * fails, you must clean up the system state (closing any open files, deleting
 * any files created in the file system, etc.) such that no trash is left
 * sitting around
 *
 * you might be wondering how to learn what system calls to use and what
 * arguments they expect. one way is to look at a web page like this one:
 * http://blog.rchapman.org/post/36801038863/linux-system-call-table-for-x86-64
 * another thing you can do is write some C code that uses standard I/O
 * functions to draw the picture and mark it as executable, compile the program
 * statically (e.g. "gcc foo.c -O -static -o foo"), and then disassemble it
 * ("objdump -d foo") and look at how the system calls are invoked, then do
 * the same thing manually using syscall()
 *
 *********************************************************************/

void draw_me(void) {
    int fp, write, close;

    const char* lena = "\
111111++I7$$7$$$777IIIIIIIIIIIIIIIIIIIIIIIIIIIIII7$I&&++11111111~~~$777777IIZO8O\n\
111+++11I$$$7$$777I7IIIIIIIIIII77IIIIIIIIIII7III777711+111111111I~:~7777II78888O\n\
11+++111I$Z$$$$$7777IIIIIIIIIIII777777IIIIIIIIII7777I11111111111II~:=$77IO88O88O\n\
1+111+11I$$$$$$$7777I7IIIIIII7111++++==77IIIIIII7777I111111111I1II1~:=778888O888\n\
+1I71+++1$Z$$$$777777IIII+1IIII11+++======777I7777$71111111111IIIII1::I88O88888O\n\
+1Z$I+++I$Z$$$$777777I7I7777IIIII11==~===~~~$77777$7111III1I11II1111+$88888O888D\n\
1$Z$1++1I$Z$$$$$777777I7777IIIIII11====~==:::,Z$$$$711117$I1I1111111888888O8OO$+\n\
$ZZ$1+++I$Z$$$$$77777I777777771II111+=+:~~~~~::$Z$$71111I8ZI11I111188O88888OZI11\n\
$$$$I+++1$Z$$$$$7777I$7$7777IIIIII11==~~~~~~~::::ZZ7111+I88$I1I11188OO8O8OOZ11++\n\
$$Z$11++1$ZZ$$$$7777777777II1I1II7====+~~~~~~~~::,O$I1117O88ZI1I188888O8O871I1++\n\
$$$$I+++I$Z$$$$$777~777777II7II7+++========~~~~~~::ZI111788Z:~::,888888O8I11+111\n\
$$$$I+++1$Z$$$$777$=$$$7IIIII$+++1=+===+=~~~~~~~~~~:I111$7~~:~~:,8O8888DI11+1111\n\
$$$$I+++1$Z$$$$$$7I17$$$$777111I+=++=1=+=====~====~~~1I~=~~~~~:::O8888OI11+11111\n\
$$$$1+++1$Z$$$$$7$:+I7$77I71II111++1++==+==+====+==+~=~:~~~~~+~:O88888I11++11111\n\
$$$$1+++1$Z$$$$$$$~+1I$$$1I+I1I1+I1++++++++++++++1==~~~=~~~~1=1:88888711++111111\n\
$$$71+++1$Z$$$$$$Z=~11I7III111I11+1111Z11+++=++~+===~~~~~~:11Z=88888$11+11111111\n\
$$$71+++1$Z$$$$$$Z:++77777IIII71717$$1IZ$O88D==~~~==~~~~:1ZZ$~$8888811++1+111111\n\
7$$71+++1$Z$$$$$$$1=+I1$II7II1IIIO78O78I78D++========~71$7++~88O8OOI1++++1+11111\n\
$$$71+++1$Z$$$$$$$$:=+$III7III7Z$O$87OZO$+7~===++===87171+=O88O888$11+++++111111\n\
7$$$I+++1$O$$$$$77$Z~~I7I77I7Z$$OO888OOZI+==+==+==~~IOZ1+O88OOO88OII++++1+++1111\n\
7$$$I+++1$Z$$$$$$77$~I77777$O88$Z$O$8D1+1=~++=+=~~~~=Z8O+888OO888II11++11+++1111\n\
$$$$I+++17Z$$$$$77777$$7I$O8$OOOOZ$I87+=1=++1+==~~~~~$8O+OOOOO88ZI11111111+11111\n\
$$$$I+++17Z$$7$7777I$+I7Z$Z8OZ888D87I++=++++++==~~~~:1O8IZOOO8881I++1++11111I111\n\
$$$$I+++1$ZZ$$$$77IIOZZ$$8O8O$8D781==+=+1I1$O1+====1Z7Z88I8O88O7I+111111111111II\n\
$$$ZI+++17O$$$$ZI111Z8Z$$78Z8DI8D+=+~=O888ODII1+~=OO8OO88IOO8O8111111111111111II\n\
$$ZOI+++17Z$7$7O1==88OZZ1I88$8O8$==~II7Z$$~1I7I=:7$$188881ZO88$I11111111111III1+\n\
$$$OI++++7Z$771+OZ8ZIZZ7$I8888D71=~$I111111+1II+:+II$$OO8=$8O8I1111111111II1====\n\
$ZZOI++++7Z$71+1ZZ$7I8O$$17888O$=+Z7I+++=+=+1II1,+1II7O8D=OO8$I111111111II======\n\
ZOOOI++++7Z$$7IZZOO$OO88887$8D7=I8$7I11+==++1II1:++1I78O8=O88I111111111II====~~~\n\
ZOO8I+++17ZI7I7$O$1+8O8OZ7=8DI~Z8Z$III1++++1II7I~:11IOOOD+IOZI+111111III====~~~~\n\
ZZO8I++++7I$7Z7$OZI+18ZO8I8D+=188$$7II1111117II7==1I788O8718II1111111II+==~~~~~~\n\
ZZOZ1++++7Z$78$8OZI1I878$Z7I7O8O87$7II1111+11I1+~+1I88OOO878111111111II==~~~~~~~\n\
OOOZ1++++7Z$7OIOZ8Z7I18O7+7$D88Z8$$III1I111111~+~1I888OO881I1111111III1=~~~~~~~~\n\
8OO$1++++77Z1O8$O$7$ZZOI:Z788O8887Z7III117Z$$$777IZ888OZO8+1111111111I+~~~~~~~~~\n\
OOO$1++=+IZ+7O8871777I+7$~OOOO8OOZO$77III1III111178O8OZ$O8+I111111111I+:~~~~~~~~\n\
O8Z$I++++7O7ZZ8$O+IIZI7I~88O888OO8OZZ$7II1111+117O88O87ZZ8+11111111II7=:~~~~~~~~\n\
88O$I++++7O7O$OZ81O$1I1$O88O8O88OOO8O8Z7I11++++18OOOOO7ZZZ++1111111II7~:~~~~~=+Z\n\
O88ZI++++7$Z8ZZZ811O711$=88Z88888O88Z$IIII1111+=~ZO8887ZZZ=I$777IIIIII~~~~~+Z88Z\n\
IZZ8$1+++77888OOZ87$1I771O$O888888OO8777III111+==~~:O8ZZZO11IIII7$$$7$~~~~18ZZZZ\n\
11IO$++++~I8OO8OOO87ZZ$I7ZI88OO8OOO$87771III1++===~~~:OOZ81111IIII777+~~=7O$Z7$$\n\
$+1O$++++1I888O88D$O$7ZZ7IO~8O88ZOO78$IIII1111++===~~::888I1111IIIIII~~~1ZZ$7Z$$\n\
81+O$++++I8888878OO88$7$I78$888OO8Z787IIIII1111++==~~:::8Z111IIIII7~~:~=$$$7$$$$\n\
81+ZZ++++1O888O$OO888OO1+7178OZ88ZII87II1III111+++=~~~::8I1IIIIII71~~:~7$$7$$$$Z\n\
8I+$Z1++++8888OOO8Z8O77$71$88O88O$I7ZIII11I1I111++==~~:::711IIIIIII=~~177$Z$$$$$\n\
DI+7Z1+++I$8888ZO8888Z$87$Z8888OZ7IO7II11IIII111+++=~~:::IZ$77III1II~~7$ZZ$$Z7Z8\n\
8$=1Z1+++$Z88OOOZOO8887I$$88888Z77O$IIII11IIII1111++=~~::~$$ZZO8OI==~IOZZ$$Z$$O8\n\
$I=+Z1+++$$888O88OZ88OZO1888D8Z77D$IIII1I111III111++=~~~::Z7$$ZO8+==7OZZ$$Z$7ZOO\n\
Z$++Z1+++$8OOO8OZ8IO$OO$=888888Z7II7IIIII111II11111+==~~::=Z$$$$$IIZZZZ7ZO$7$OO8\n\
8Z~~71==+Z88OOO8I8II88O8$888Z$7777IIIIIII1111111111++==~~::Z$77777$7$$$ZZ$77OOOZ\n\
88~=71+=+$8Z8OZO$8IZ7$8Z88IZ$777I7IIIIIIII111111111+++=~~~:I$$$I77I7ZZ$Z$I7O8O$7\n";

    // option val ref: http://unix.superglobalmegacorp.com/Net2/newsrc/sys/fcntl.h.html
    // stat val ref: http://unix.superglobalmegacorp.com/Net2/newsrc/sys/stat.h.html
    fp = syscall(SYS_open, "./me.txt", 0x642, 0777);
    if (fp != 0) {
        for (int i = 0; i < 50 * 81; i++) {
            write = syscall(SYS_write, fp, &lena[i], 1);
            if (write <= 0) {
                syscall(SYS_unlink, "./me.txt");
                return;
            }
        }
        close = syscall(SYS_close, fp);
        if (close != 0) syscall(SYS_unlink, "./me.txt");
    }
    return;
}
