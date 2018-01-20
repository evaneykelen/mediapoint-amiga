/*
 * File: test.rexx
 *
 * SimpleRexx test...
 *
 * You need to run the SimpleRexxExample first...
 */

Options FailAt 100

Options Results

/*
 * Try to read the window title bar
 */
Address EXAMPLE.1 ReadTitle

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window title is 'Result

/*
 * Bad WINDOW command...
 */
Address EXAMPLE.1 "Window Display"

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window is now open'

/*
 * Open the window
 */
Address EXAMPLE.1 "Window Open"

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window is now open'

/*
 * Open the window
 */
Address EXAMPLE.1 "Window Open"

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window is now open'

/*
 * Try to read the window title bar
 */
Address EXAMPLE.1 ReadTitle

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window title is 'Result

/*
 * Hide the window
 */
Address EXAMPLE.1 "Window Close"

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window is now closed'

/*
 * Try to hide the window again
 */
Address EXAMPLE.1 "Window Close"

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'Window is now closed'

/*
 * Send a command that does not exist
 */
Address EXAMPLE.1 Junk

if rc > 0 then say 'Error was 'EXAMPLE.LASTERROR
else say 'The command worked!!!'

/*
 * Quit the program...
 */
Address EXAMPLE.1 Quit
