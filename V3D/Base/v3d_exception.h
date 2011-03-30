// -*- C++ -*-
/*
Copyright (c) 2008-2010 UNC-Chapel Hill & ETH Zurich

This file is part of GPU-KLT+FLOW.

GPU-KLT+FLOW is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

GPU-KLT+FLOW is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with GPU-KLT+FLOW. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef V3D_EXCEPTION_H
#define V3D_EXCEPTION_H

#include <exception>
#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>

#define verify(condition, message) do                      \
   {                                                       \
      if (!(condition)) {                                  \
         std::cout << "VERIFY FAILED: " << (message) << "\n"       \
              << "    " << __FILE__ << ", " << __LINE__ << "\n"; \
         assert(false);                                    \
         exit(0);                                          \
      }                                                    \
   } while(false);

#define throwV3DErrorHere(reason) throw V3D::Exception(__FILE__, __LINE__, reason)

namespace V3D
{

   struct Exception : public std::exception
   {
         Exception(char const * reason)
            : _reason(reason)
         { }

         Exception(std::string const& reason)
            : _reason(reason)
         { }

         Exception(char const * file, int line, char const * reason)
         {
            std::ostringstream os;
            os << file << ":" << line << ": " << reason;
            _reason = os.str();
         }

         Exception(char const * file, int line, std::string const& reason)
         {
            std::ostringstream os;
            os << file << ":" << line << ": " << reason;
            _reason = os.str();
         }

         virtual ~Exception() throw() { }

         virtual const char * what() const throw()
         {
            return _reason.c_str();
         }

      protected:
         std::string _reason;
   }; // end struct Exception

} // end namespace V3D

#endif
