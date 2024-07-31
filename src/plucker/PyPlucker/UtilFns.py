# -*- Python -*-

"""

UtilFns.py   $Id: UtilFns.py,v 1.3 2002/05/18 10:28:24 nordstrom Exp $


Copyright 2001 by Bill Janssen <bill@janssen.org>

Distributable under the GNU General Public License Version 2 or newer.
"""

import sys, traceback

MessageStream = sys.stderr
ErrorStream = sys.stderr

CurrentVerbosityLevel = 1

def set_verbosity(level):
    global CurrentVerbosityLevel
    CurrentVerbosityLevel = int(level)


def show_exception(level=1, exn=None):
    """Usage:  show_exception (VERBOSITY_LEVEL (defaults to 1), EXCEPTION (defaults to sys.exc)).
    Prints the indicated exception to UtilFns.ErrorStream if CurrentVerbosityLevel >= VERBOSITY_LEVEL."""
    if not exn:
        etype, evalue, etb = sys.exc_type, sys.exc_value, sys.exc_traceback
    else:
        etype, evalue, etb = exn
    if level <= CurrentVerbosityLevel:
        traceback.print_exception(etype, evalue, etb, 20, ErrorStream)


def message(*args):
    """Usage:  message ([MIN_VERBOSITY_LEVEL (defaults to 1), ] FORMAT_STRING [, ARGS...])
    If CurrentVerbosityLevel is greater than or equal to MIN_VERBOSITY_LEVEL, will write
    the message to UtilFns.MessageStream (which defaults to sys.stderr)."""
    if len(args) < 1:
        raise ValueError("call to 'message' with no arguments")
    if type(args[0]) == type(2):
        verbosity_level = args[0]
        args = args[1:]
    else:
        verbosity_level = 1
    if not type(args[0]) == type(''):
        raise ValueError("call to 'message' with no format argument -- " + str(args[0]) + " found instead")
    if len(args) > 1:
        actual_message = args[0] % args[1:]
    else:
        actual_message = args[0]
    if len(actual_message) < 1 or actual_message[-1] != '\n':
        actual_message = actual_message + '\n'
    if verbosity_level <= CurrentVerbosityLevel:
        MessageStream.write(actual_message)
        MessageStream.flush()


def error(*args):
    """Usage:  error (FORMAT_STRING [, ARGS...])
    Writes message to UtilFns.ErrorStream, which by default is sys.stderr."""
    if not type(args[0]) == type(''):
        raise ValueError("call to 'error' with no format argument -- " + str(args[0]) + " found instead")
    if len(args) > 1:
        actual_message = 'Error:  ' + args[0] % args[1:]
    else:
        actual_message = 'Error:  ' + args[0]
    if len(actual_message) < 1 or actual_message[-1] != '\n':
        actual_message = actual_message + '\n'
    ErrorStream.write(actual_message)


