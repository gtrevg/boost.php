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

#ifndef BOOST_PHP_DETAIL_OBJECT_RETRIEVER_HPP
#define BOOST_PHP_DETAIL_OBJECT_RETRIEVER_HPP

#include <zend.h>
#include <boost/php/detail/signature.hpp>

namespace boost { namespace php {

// must be specialized somewhere
template<typename Tobj_>
class object_retriever {
public:
    Tobj_* operator()(INTERNAL_FUNCTION_PARAMETERS) const;
};

template<>
class object_retriever<void> {
public:
    void* operator()(INTERNAL_FUNCTION_PARAMETERS) const { return 0; }
};

namespace detail {
    template<typename Tsig_>
    const object_retriever<typename Tsig_::object_type>&
    get_object_retriever(const Tsig_& sig) {
        static object_retriever<typename Tsig_::object_type> oretr;
        return oretr;
    }
} // namespace detail

} } // namespace boost::php

#endif /* BOOST_PHP_DETAIL_OBJECT_RETRIEVER_HPP */
