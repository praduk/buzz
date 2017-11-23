BUZZ CHESS ENGINE-README
==================================
Version         : Buzz OpenSource
Release Date    : 5/11/2008
==================================

Table of Contents
=================

1) General Description
2) Minimum System Requirements
3) Operating Instructions
	I.   Winboard Client
	II.  Console Operation
	III. Command-line parameters
4) License
5) Contact/Download Information



1) General Description
======================
Buzz is a chess engine supporting both Xboard/Winboard I and Xboard/Winboard II
protocols.

2) Minimum System Requirements
==============================

Processor   : x86 or x86-64 processor
RAM         : 8MB
OS          : Win 98, Me, NT, 2000, XP, Vista and POSIX (UNIX)

3) Operating Instructions
=========================

There are two ways to use Buzz, using a Winboard client(I), or through
console(II).

I. Winboard Client

There are many Winboard clients and each one has its own operating instructions.
Please visit http://buzzchess.webhop.org/ for links to some Winboard clients.

Buzz is not garunteed to work well under the ChessBase/Fritz GUI.  This is
because ChessBase/Fritz GUI does not implement the Winboard Protocol exactly as
defined by Tim Mann.

II. Console Operation

If you would like to operate Buzz through console you will have to know or be
able to reference the Winboard Protocol.  Type help in console to see a list of
additional Buzz-specific commands.

The Winboard protocol reference can be found at
http://www.tim-mann.org/xboard/engine-intf.html .

Moves can be sent in either Short Algebric Notation (Bb4,Ngxf6,e8=Q,O-O-O) or
Coordinate Notation (e2e4).

III. Command-line parameters

Book-Making Options
-------------------
Buzz makebook <name> <plys> <games> <razor %> <pgnFile1> <pgnFile2> ... <pgnFileN>
     <name>      - name/location of the book file
     <plys>      - how many plys to use in the book
     <games>     - add only positions with atleast <games> games
     <razor %>   - positions that score below razor% will get pruned
     <pgnFileX>  - the locations of the pgn files to use

Example: Buzz makebook grand.book 30 10 40 GM2001.pgn Euwe.pgn Massive.pgn
Example: Buzz makebook "buzz books/grand.book" 60 20 35.5 pgns/Massive.pgn

Regular Options
---------------
Both - and / will be accepted as option indicators
-bk <file>, -book <file>
     Gives the location of the book file to use.
     Example: -book small.book
     Example: -book "mybooks/book with space.book"
     Default: default.book
-h, -help
     Show the command-line help.  This will only work by itself.
-hash <megabytes>
     Set the amout of memory to use for the hash table in megabytes.
     Example: -hash 64
     Default: 64 MB
-icstalk <kibitz|whisper|adaptive|notalk>
     Determine the manner in which analysis is given on an ics.
     Parameter kibitz  : kibitz always
     Parameter whisper : whisper always
     Parameter adaptive: kibitz to computer, whisper to human
     Parameter notalk  : no analysis information given
     Example: -icstalk adaptive
     Default: adaptive
-l, -log
     Buzz will write to a log named MMDDYY_HHMMSS_log.html.
     MMDDYY_HHMMSS represents the time of creation where
     MM=month, DD=day, YY=year, HH=hour, MM=minute, and SS=second.

4) License
==========

Buzz is provided "as-is" without any express or implied warranty. In no event
will I (Pradyumna Kannan) be held liable for any damages arising from the use
of Buzz.

You may redistribute unmodified executables freely as long as this readme file
remains with the executables.

5) Contact/Download Information
===============================

Author:   Pradu Kannan
Website:  http://buzzchess.webhop.org/
Email:    praduk@gmail.com
