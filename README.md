Kilo
===

Kilo is a small text editor in less than 1K lines of code (counted with cloc).

A screencast is available here: https://asciinema.org/a/90r2i9bq8po03nazhqtsifksb

Usage: kilo `<filename>` (Depreciated for CPD project)

Server Usage: server `<port>`

Kilo Usage: kilo `<host>` `<port>` `<filename>`

Server must be started before starting kilo. Multiple users may connect to the same file
or different files.

Test Files:
- test - a simple text file with sample text
- test.cpp - a simple c++ file to show syntax highlighting

Editor Keys:

    CTRL-S: Save (concurrent version automatically saves on server) (Disabled for CPD project)
    CTRL-Q: Quit
    CTRL-F: Find string in file (ESC to exit search, arrows to navigate)

Kilo does not depend on any library (not even curses). It uses fairly standard
VT100 (and similar terminals) escape sequences. The project is in alpha
stage and was written in just a few hours taking code from my other two
projects, load81 and linenoise.

People are encouraged to use it as a starting point to write other editors
or command line interfaces that are more advanced than the usual REPL
style CLI.

Kilo was written by Salvatore Sanfilippo aka antirez and is released
under the BSD 2 clause license.

Concurrent Text File Editing
===

Term Project for COP5570

Contributers:
Skylar Scorca,
Tony Drouillard,
Jack Dewey

Features added:
- Added a server to receive and forward update commands
- Kilo client connects to server and does not save the file locally
- Kilo allows users to edit the file and view the edits of other users simultaneously
- Server automatically saves the file contents for every update
