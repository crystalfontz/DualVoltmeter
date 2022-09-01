REM Usage: C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-objdump.exe <option(s)> <file(s)>
REM  Display information from object <file(s)>.
REM  At least one of the following switches must be given:
REM   -a, --archive-headers    Display archive header information
REM   -f, --file-headers       Display the contents of the overall file header
REM   -p, --private-headers    Display object format specific file header contents
REM   -P, --private=OPT,OPT... Display object format specific contents
REM   -h, --[section-]headers  Display the contents of the section headers
REM   -x, --all-headers        Display the contents of all headers
REM   -d, --disassemble        Display assembler contents of executable sections
REM   -D, --disassemble-all    Display assembler contents of all sections
REM   -S, --source             Intermix source code with disassembly
REM   -s, --full-contents      Display the full contents of all sections requested
REM   -g, --debugging          Display debug information in object file
REM   -e, --debugging-tags     Display debug information using ctags style
REM   -G, --stabs              Display (in raw form) any STABS info in the file
REM   -W[lLiaprmfFsoRt] or
REM   --dwarf[=rawline,=decodedline,=info,=abbrev,=pubnames,=aranges,=macro,=frames,
REM           =frames-interp,=str,=loc,=Ranges,=pubtypes,
REM           =gdb_index,=trace_info,=trace_abbrev,=trace_aranges,
REM           =addr,=cu_index]
REM                            Display DWARF info in the file
REM   -t, --syms               Display the contents of the symbol table(s)
REM   -T, --dynamic-syms       Display the contents of the dynamic symbol table
REM   -r, --reloc              Display the relocation entries in the file
REM   -R, --dynamic-reloc      Display the dynamic relocation entries in the file
REM   @<file>                  Read options from <file>
REM   -v, --version            Display this program's version number
REM   -i, --info               List object formats and architectures supported
REM   -H, --help               Display this information
REM 
REM  The following switches are optional:
REM   -b, --target=BFDNAME           Specify the target object format as BFDNAME
REM   -m, --architecture=MACHINE     Specify the target architecture as MACHINE
REM   -j, --section=NAME             Only display information for section NAME
REM   -M, --disassembler-options=OPT Pass text OPT on to the disassembler
REM   -EB --endian=big               Assume big endian format when disassembling
REM   -EL --endian=little            Assume little endian format when disassembling
REM       --file-start-context       Include context from start of file (with -S)
REM   -I, --include=DIR              Add DIR to search list for source files
REM   -l, --line-numbers             Include line numbers and filenames in output
REM   -F, --file-offsets             Include file offsets when displaying information
REM   -C, --demangle[=STYLE]         Decode mangled/processed symbol names
REM                                   The STYLE, if specified, can be `auto', `gnu',
REM                                   `lucid', `arm', `hp', `edg', `gnu-v3', `java'
REM                                   or `gnat'
REM   -w, --wide                     Format output for more than 80 columns
REM   -z, --disassemble-zeroes       Do not skip blocks of zeroes when disassembling
REM       --start-address=ADDR       Only process data whose address is >= ADDR
REM       --stop-address=ADDR        Only process data whose address is <= ADDR
REM       --prefix-addresses         Print complete address alongside disassembly
REM       --[no-]show-raw-insn       Display hex alongside symbolic disassembly
REM       --insn-width=WIDTH         Display WIDTH bytes on a single line for -d
REM       --adjust-vma=OFFSET        Add OFFSET to all displayed section addresses
REM       --special-syms             Include special symbols in symbol dumps
REM       --prefix=PREFIX            Add PREFIX to absolute paths for -S
REM       --prefix-strip=LEVEL       Strip initial directory names for -S
REM       --dwarf-depth=N        Do not display DIEs at depth N or greater
REM       --dwarf-start=N        Display DIEs starting with N, at the same depth
REM                              or deeper
REM       --dwarf-check          Make additional dwarf internal consistency checks.

REM Usage: C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-nm.exe [option(s)] [file(s)]
REM  List symbols in [file(s)] (a.out by default).
REM  The options are:
REM   -a, --debug-syms       Display debugger-only symbols
REM   -A, --print-file-name  Print name of the input file before every symbol
REM   -B                     Same as --format=bsd
REM   -C, --demangle[=STYLE] Decode low-level symbol names into user-level names
REM                           The STYLE, if specified, can be `auto' (the default),
REM                           `gnu', `lucid', `arm', `hp', `edg', `gnu-v3', `java'
REM                           or `gnat'
REM       --no-demangle      Do not demangle low-level symbol names
REM   -D, --dynamic          Display dynamic symbols instead of normal symbols
REM       --defined-only     Display only defined symbols
REM   -e                     (ignored)
REM   -f, --format=FORMAT    Use the output format FORMAT.  FORMAT can be `bsd',
REM                            `sysv' or `posix'.  The default is `bsd'
REM   -g, --extern-only      Display only external symbols
REM   -l, --line-numbers     Use debugging information to find a filename and
REM                            line number for each symbol
REM   -n, --numeric-sort     Sort symbols numerically by address
REM   -o                     Same as -A
REM   -p, --no-sort          Do not sort the symbols
REM   -P, --portability      Same as --format=posix
REM   -r, --reverse-sort     Reverse the sense of the sort
REM       --plugin NAME      Load the specified plugin
REM   -S, --print-size       Print size of defined symbols
REM   -s, --print-armap      Include index for symbols from archive members
REM       --size-sort        Sort symbols by size
REM       --special-syms     Include special symbols in the output
REM       --synthetic        Display synthetic symbols as well
REM   -t, --radix=RADIX      Use RADIX for printing symbol values
REM       --target=BFDNAME   Specify the target object format as BFDNAME
REM   -u, --undefined-only   Display only undefined symbols
REM   -X 32_64               (ignored)
REM   @FILE                  Read options from FILE
REM   -h, --help             Display this information
REM   -V, --version          Display this program's version number



echo ========== avr-size ========== > map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-size.exe" "D:\Arduino_Build\CFAG12864T3U3.ino.elf" >> map.txt
echo ========== avr-nm ========== >> map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-nm.exe"  -nS "D:\Arduino_Build\CFAG12864T3U3.ino.elf" >> map.txt
echo ========== .bss only ========== >> map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-objdump.exe"  -t --section=.bss "D:\Arduino_Build\CFAG12864T3U3.ino.elf" >> map.txt
echo ========== .text only ========== >> map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-objdump.exe"  -t --section=.text "D:\Arduino_Build\CFAG12864T3U3.ino.elf" >> map.txt
echo ========== default ========== >> map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-objdump.exe"  -t "D:\Arduino_Build\CFAG12864T3U3.ino.elf" >> map.txt
echo ========== done ========== >> map.txt
