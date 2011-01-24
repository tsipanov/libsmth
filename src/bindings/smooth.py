# Copyright (C) 2010 Stefano Sanfilippo
#
# smooth.py : Python libsmth bindings
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

import ctypes

libsmth = ctypes.CDLL('libsmth.so.0')

string = ctypes.c_char_p
voidp  = ctypes.c_void_p
size_t = ctypes.c_int

open = libsmth.SMTH_open
open.argtypes = [string, string]
open.restype  = voidp

read = libsmth.SMTH_read
read.argtypes = [voidp, size_t, size_t, voidp]
read.restype  = size_t

eos  = libsmth.SMTH_EOS
eos.argtypes = [voidp, size_t]
eos.restypes = size_t

close = libsmth.SMTH_close
close.argtypes = [voidp]
close.restypes = None

#void SMTH_getinfo(SMTH_setting what, SMTHh handle, ...);
