//######################################################################
//
// GraspIt!
// Copyright (C) 2002-2009  Columbia University in the City of New York.
// All rights reserved.
//
// GraspIt! is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GraspIt! is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GraspIt!.  If not, see <http://www.gnu.org/licenses/>.
//
// Author(s): Matei T. Ciocarlie
//
// $Id: debug.h,v 1.9.4.1 2009/07/23 21:17:40 cmatei Exp $
//
//######################################################################

/*! \file
	Defines some convenience macros for output and debug, which behave
	differently depending on whether GRASPITDBG is defined:

	DBGA(msg) always prints the message to std::err

	DBGP(msg) only prints the message if GRASPITDBG is defined, and
	swallows it otherwise.

	To use this, include "debug.h" in your source file, and then
	define GRASPITDBG just before the #include if you want the debug
	output.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <iostream>
#include <sstream>

#define VERBOSE

inline std::string PRINT_PREFIX(const std::string &fname, long lineno) {
    std::string debug_file_path(__FILE__); // expands to GRASPIT_ROOT/include/debug.h
    size_t include_path_pos = debug_file_path.find_last_of("/");
    size_t graspit_path_pos = debug_file_path.find_last_of("/", include_path_pos - 1);
    std::string graspit_root = debug_file_path.substr(0, graspit_path_pos);

    size_t idx = fname.find(graspit_root);

    std::ostringstream oss;
    if (idx != std::string::npos) {
        oss << fname.substr(graspit_root.length() + 1);
    } else {
        oss << fname;
    }
    oss << ":" << lineno << "| ";
    return oss.str();
}

#ifdef GRASPITDBG
#define DBGP(STMT) std::cerr << "\033[33m" << PRINT_PREFIX(__FILE__,__LINE__) << "\033[0m" << STMT << std::endl;
#define DBGST(STMT) STMT;
#else
#define DBGP(STMT)
#define DBGST(STMT)
#endif

#ifdef VERBOSE
#define DBGA(STMT) std::cerr << "\033[33m" << PRINT_PREFIX(__FILE__,__LINE__) << "\033[0m" << STMT << std::endl;
#else
#define DBGA(STMT) std::cerr << STMT << std::endl;
#endif

#define PRINT_STAT(STREAM_PTR, STMT) if (STREAM_PTR) *STREAM_PTR << STMT << " ";
#define DBGAF(STREAM, STMT) {STREAM<<STMT<<std::endl; DBGA(STMT)}
#define DBGPF(STREAM, STMT) {STREAM<<STMT<<std::endl; DBGP(STMT)}

#endif
