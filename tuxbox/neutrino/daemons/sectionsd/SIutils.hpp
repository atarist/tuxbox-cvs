#ifndef SIUTILS_HPP
#define SIUTILS_HPP
//
// $Id: SIutils.hpp,v 1.1 2001/05/16 15:23:47 fnbrd Exp $
//
// utility functions for the SI-classes (dbox-II-project)
//
//    Homepage: http://dbox2.elxsi.de
//
//    Copyright (C) 2001 fnbrd (fnbrd@gmx.de)
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// $Log: SIutils.hpp,v $
// Revision 1.1  2001/05/16 15:23:47  fnbrd
// Alles neu macht der Mai.
//
//

time_t changeUTCtoCtime(const unsigned char *buffer);

// returns the descriptor type as readable text
const char *decode_descr (unsigned char tag_value);

#endif // SIUTILS_HPP
