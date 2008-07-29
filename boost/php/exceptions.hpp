//
// Copyright 2008 (C) Moriyoshi Koizumi. All rights reserved.
//
// This software is distributed under the Boost Software License, Version 1.0.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <string>
#include <stdexcept>
#include <zend.h>
#include <boost/php/error.hpp>

#ifndef BOOST_PHP_RUNTIME_ERROR_DEFINED
#define BOOST_PHP_RUNTIME_ERROR_DEFINED

namespace boost { namespace php {
class runtime_error: public ::std::runtime_error {
public:
    runtime_error(const error_info& err);

    runtime_error(const ::std::string msg,
        const char* filename = 0, const ::uint lineno = 0);

    virtual ~runtime_error() throw() {}

    const ::std::string& filename() const {
        return filename_;
    }

    const ::uint line_number() const {
        return line_number_;
    }

protected:
    const ::std::string filename_;
    const ::uint line_number_;
};

} } // namespace boost::php

#endif /* BOOST_PHP_RUNTIME_ERROR_DEFINED */

#ifndef BOOST_PHP_MISC_ERRORS_DEFINED
#define BOOST_PHP_MISC_ERRORS_DEFINED

namespace boost { namespace php {

class arithmetic_error: public runtime_error {
public:
    arithmetic_error(const error_info& err)
        : runtime_error(err) {}

    arithmetic_error(const ::std::string msg,
            const char* filename = 0, const ::uint lineno = 0)
        : runtime_error(msg, filename, lineno) {}

    virtual ~arithmetic_error() throw() {}
};

class type_error: public runtime_error {
public:
    type_error(const error_info& err)
        : runtime_error(err) {}

    type_error(const ::std::string msg,
            const char* filename = 0, const ::uint lineno = 0)
        : runtime_error(msg, filename, lineno) {}

    virtual ~type_error() throw() {}
};

class not_found: public runtime_error {
public:
    not_found(const error_info& err)
        : runtime_error(err) {}

    not_found(const ::std::string msg,
            const char* filename = 0, const ::uint lineno = 0)
        : runtime_error(msg, filename, lineno) {}

    virtual ~not_found() throw() {}
};

class illegal_argument: public runtime_error {
public:
    illegal_argument(const error_info& err)
        : runtime_error(err) {}

    illegal_argument(const ::std::string msg,
            const char* filename = 0, const ::uint lineno = 0)
        : runtime_error(msg, filename, lineno) {}

    virtual ~illegal_argument() throw() {}
};

} } // namespace boost::php

#endif /* BOOST_PHP_MISC_ERRORS_DEFINED */

#include <boost/php/utils.hpp>

#ifndef BOOST_PHP_RUNTIME_ERROR_MEMBERS_DEFINED
#define BOOST_PHP_RUNTIME_ERROR_MEMBERS_DEFINED

namespace boost { namespace php {

inline runtime_error::runtime_error(const error_info& err)
    : filename_(err.filename()), line_number_(err.line_number()),
      ::std::runtime_error(err.message()) {}

inline runtime_error::runtime_error(const ::std::string msg,
    const char* filename, const ::uint lineno)
    : filename_(filename ? filename: utils::current_filename()),
      line_number_(filename ? lineno:
            ::boost::php::utils::current_line_number()),
      ::std::runtime_error(msg) {}

} } // namespace boost::php

#endif /* BOOST_PHP_RUNTIME_ERROR_MEMBERS_DEFINED */
