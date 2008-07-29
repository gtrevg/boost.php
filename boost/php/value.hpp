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

#include <cstddef>
#include <cstring>
#include <limits>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <zend.h>
#include <zend_variables.h>
#include <zend_operators.h>
#include <boost/php/string.hpp>
#include <boost/php/resource_handle.hpp>
#include <boost/php/detail/tsrm_macros.hpp>

#ifndef BOOST_PHP_VALUE_DEFINED
#define BOOST_PHP_VALUE_DEFINED

#if ZEND_MODULE_API_NO < 20071006
#define BOOST_PHP_ZVAL_ASSERT_REFCOUNT(p) \
        BOOST_ASSERT((p)->refcount >= 1)
#define BOOST_PHP_ZVAL_ADDREF(p) ZVAL_ADDREF(p)
#else
#define BOOST_PHP_ZVAL_ASSERT_REFCOUNT(p) \
        BOOST_ASSERT(Z_REFCOUNT_P(p) >= 1)
#define BOOST_PHP_ZVAL_ADDREF(p) Z_ADDREF_P(p)
#endif /* ZEND_MODULE_API_NO < 20071006 */

namespace boost { namespace php {

class value_ptr;
class symtable_key;
template<typename Telem_, typename Tkey_> class hashtable;
typedef hashtable<value_ptr, symtable_key> array;

class value: public ::zval {
public:
    enum typecode {
        _NULL = IS_NULL,
        _LONG = IS_LONG,
        _DOUBLE = IS_DOUBLE,
        _BOOL = IS_BOOL,
        _ARRAY = IS_ARRAY,
        _OBJECT = IS_OBJECT,
        _STRING = IS_STRING,
        _RESOURCE = IS_RESOURCE
    };
public:
    value(): zval(::zval_used_for_init) {}

    value(const ::zval& that): ::zval(that) {
        zval_copy_ctor(static_cast<zval*>(this));
        initialize();
    }

    value(const value& that): ::zval(that) {
        zval_copy_ctor(static_cast<zval*>(this));
        initialize();
    }

    value(const ::zval* that): ::zval(*that) {
        zval_copy_ctor(static_cast<zval*>(this));
        initialize();
    }

    value(const value* that): ::zval(*that) {
        zval_copy_ctor(static_cast<zval*>(this));
        initialize();
    }

    value(::zval const& that, enum typecode desired_type);

    value(value_ptr const& that);

    value(bool v) {
        ::zval::type = IS_BOOL;
        ::zval::value.lval = ZEND_NORMALIZE_BOOL(v);
        initialize();
    }

    value(int v);

    value(long v) {
        ::zval::type = IS_LONG;
        ::zval::value.lval = v;
        initialize();
    }

    value(double v) {
        ::zval::type = IS_DOUBLE;
        ::zval::value.dval = v;
        initialize();
    }

    value(char const* v) {
        ::zval::type = IS_STRING;
        ::zval::value.str.len = ::strlen(v);
        ::zval::value.str.val = ::estrndup(v, ::zval::value.str.len);
        initialize();
    }

    value(string const& v) {
        ::zval::type = IS_STRING;
        ::zval::value.str.val = ::estrndup(v, v.size());
        ::zval::value.str.len = v.size();
        initialize();
    }

    value(::std::string const& v) {
        ::zval::type = IS_STRING;
        ::zval::value.str.len = v.size();
        ::zval::value.str.val = ::estrndup(v.data(), v.size());
        initialize();
    }

    value(resource_handle const& v) {
        ::zval::type = IS_RESOURCE;
        ::zval::value.lval = v;
        initialize();
    }

    value(zend_object_value const& v) {
        ::zval::type = IS_OBJECT;
        ::zval::value.obj = v;
        initialize();
    }

    value(array const& v);

    value(array* v);

    ~value() {
        zval_dtor(static_cast<zval*>(this));
    }

    void swap(::zval& that) throw() {
        zval tmp;
        tmp = that;
        that = *static_cast< ::zval*>(this);
        *static_cast< ::zval*>(this) = tmp;
    }

    const enum typecode typecode() const {
        return static_cast<enum typecode>(::zval::type);
    }

    const bool is_null() const {
        return ::zval::type == IS_NULL;
    }

    const value concat(zval const& rhs TSRMLS_DC) const;
    const value is_equal(zval const& rhs TSRMLS_DC) const;
    const value is_not_equal(::zval const& rhs TSRMLS_DC) const;
    value& increment();
    value& decrement();
    const value add(::zval const& rhs TSRMLS_DC) const;
    const value sub(::zval const& rhs TSRMLS_DC) const;
    const value mul(::zval const& rhs TSRMLS_DC) const;
    const value div(::zval const& rhs TSRMLS_DC) const;
    const value mod(::zval const& rhs TSRMLS_DC) const;
    const value bitwise_or(::zval const& rhs TSRMLS_DC) const;
    const value bitwise_and(::zval const& rhs TSRMLS_DC) const;
    const value bitwise_xor(::zval const& rhs TSRMLS_DC) const;
    const value shift_left(::zval const& rhs TSRMLS_DC) const;
    const value shift_right(::zval const& rhs TSRMLS_DC) const;
    const value is_smaller(::zval const& rhs TSRMLS_DC) const;
    const value is_greater_or_equal(::zval const& rhs TSRMLS_DC) const;
    const value is_smaller_or_equal(::zval const& rhs TSRMLS_DC) const;
    const value is_greater(::zval const& rhs TSRMLS_DC) const;
    const value is_identical(::zval const& rhs TSRMLS_DC) const;
    const value is_not_identical(::zval const& rhs TSRMLS_DC) const;
#ifdef ZTS
    const value boolean_or(::zval const& rhs TSRMLS_DC = 0) const;
    const value boolean_and(::zval const& rhs TSRMLS_DC = 0) const;
#else
    const value boolean_or(::zval const& rhs) const;
    const value boolean_and(::zval const& rhs) const;
#endif
    const value boolean_not(TSRMLS_D) const;
    const value bitwise_not(TSRMLS_D) const;

#ifdef ZTS
    const value concat(::zval const& rhs) const {
        return concat(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_equal(::zval const& rhs) const {
        return is_equal(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_not_equal(::zval const& rhs) const {
        return is_not_equal(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value add(::zval const& rhs) const {
        return add(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value mul(::zval const& rhs) const {
        return mul(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value sub(::zval const& rhs) const {
        return sub(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value div(::zval const& rhs) const {
        return div(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value mod(::zval const& rhs) const {
        return mod(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value bitwise_or(::zval const& rhs) const {
        return bitwise_or(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value bitwise_and(::zval const& rhs) const {
        return bitwise_and(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value bitwise_xor(::zval const& rhs) const {
        return bitwise_xor(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value shift_right(::zval const& rhs) const {
        return shift_right(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value shift_left(::zval const& rhs) const {
        return shift_left(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_smaller(::zval const& rhs) const {
        return is_smaller(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_greater_or_equal(::zval const& rhs) const {
        return is_greater_or_equal(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_smaller_or_equal(::zval const& rhs) const {
        return is_smaller_or_equal(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_greater(::zval const& rhs) const {
        return is_greater(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_identical(::zval const& rhs) const {
        return is_identical(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value is_not_identical(::zval const& rhs) const {
        return is_not_identical(rhs BOOST_PHP_TSRM_DIRECT_CC);
    }

    const value boolean_not() const {
        return boolean_not(BOOST_PHP_TSRM_DIRECT_C);
    }

    const value bitwise_not() const {
        return bitwise_not(BOOST_PHP_TSRM_DIRECT_C);
    }
#endif /* ZTS */

    value& operator++() {
        return increment();
    }

    const value operator++(int) {
        value retval(*this);
        increment();
        return retval;
    }

    value& operator--() {
        decrement();
        return *this;
    }

    const value operator--(int) {
        value retval(*this);
        decrement();
        return retval;
    }

    const value operator+(::zval const& rhs) const {
        return add(rhs);
    }

    const value operator-(::zval const& rhs) const {
        return sub(rhs);
    }

    const value operator*(::zval const& rhs) const {
        return mul(rhs);
    }

    const value operator/(::zval const& rhs) const {
        return div(rhs);
    }

    const value operator%(::zval const& rhs) const {
        return mod(rhs);
    }

    const value operator|(::zval const& rhs) const {
        return bitwise_or(rhs);
    }

    const value operator&(::zval const& rhs) const {
        return bitwise_and(rhs);
    }

    const value operator^(::zval const& rhs) const {
        return bitwise_xor(rhs);
    }

    const value operator<<(::zval const& rhs) const {
        return shift_left(rhs);
    }

    const value operator>>(::zval const& rhs) const {
        return shift_right(rhs);
    }

    const value operator<(::zval const& rhs) const {
        return is_smaller(rhs);
    }

    const value operator>=(::zval const& rhs) const {
        return is_greater_or_equal(rhs);
    }

    const value operator<=(::zval const& rhs) const {
        return is_smaller_or_equal(rhs);
    }

    const value operator>(::zval const& rhs) const {
        return is_greater(rhs);
    }

    const value operator==(::zval const& rhs) const {
        return is_identical(rhs);
    }

    const bool operator==(const bool& rhs) const {
        return type == _BOOL && !!::zval::value.lval == rhs;
    }

    const value operator!=(::zval const& rhs) const {
        return is_not_identical(rhs);
    }

    const bool operator!=(const bool& rhs) const {
        return type == _BOOL && !!::zval::value.lval != rhs;
    }

    const value operator||(::zval const& rhs) const {
        return boolean_or(rhs);
    }

    const value operator&&(::zval const& rhs) const {
        return boolean_and(rhs);
    }

    const value operator!() const {
        return boolean_not();
    }

    const value operator~() const {
        return bitwise_not();
    }

    operator string&();

    operator const string&() const;

    operator array&();

    operator const array&() const;

    operator ::std::string() const;

    operator resource_handle() const;

    operator long&();

    operator long() const;

    operator double&();

    operator double const&() const;

    operator ::zend_object_value const&() const;

    static const char* get_type_string(int t) {
        return get_type_string(static_cast<enum typecode>(t));
    }

    static const char* get_type_string(enum typecode t) {
        switch (t) {
        case _NULL:
            return "null";
        case _LONG:
            return "integer";
        case _DOUBLE:
            return "double";
        case _BOOL:
            return "boolean";
        case _ARRAY:
            return "array";
        case _OBJECT:
            return "object";
        case _STRING:
            return "string";
        case _RESOURCE:
            return "resource";
        }
        return "unknown";
    }

    static void* operator new(::std::size_t sz) {
        zval* retval;
        ALLOC_ZVAL(retval);
        return operator new(sz, retval);
    }

    static void* operator new(::std::size_t sz, void *retval) {
        BOOST_ASSERT(sz == sizeof(zval));
        return retval;
    }

    static void operator delete(void* p) {
        FREE_ZVAL(reinterpret_cast<zval*>(p));
    }

private:
    void initialize() {
        INIT_PZVAL(this);
    }
};

#endif /* BOOST_PHP_VALUE_DEFINED */

#ifndef BOOST_PHP_VALUE_PTR_DEFINED
#define BOOST_PHP_VALUE_PTR_DEFINED
class value_ptr {
public:
    value_ptr(): p_(0)
    {
        TSRMLS_FETCH();
        p_ = reinterpret_cast<value*>(&EG(uninitialized_zval));
        BOOST_PHP_ZVAL_ADDREF(p_);
    }

    value_ptr(::zval* p): p_(static_cast<value*>(p)) {
        BOOST_PHP_ZVAL_ADDREF(p_);
    }

    value_ptr(::zval* p, bool add_ref): p_(static_cast<value*>(p)) {
        if (add_ref) {
            BOOST_PHP_ZVAL_ADDREF(p_);
        }
    }

    value_ptr(::zval*const* that): p_(static_cast<value*>(*that)) {
        BOOST_PHP_ZVAL_ASSERT_REFCOUNT(p_);
        BOOST_PHP_ZVAL_ADDREF(p_);
    }

    value_ptr(value_ptr const& that): p_(that.p_) {
        BOOST_PHP_ZVAL_ASSERT_REFCOUNT(p_);
        BOOST_PHP_ZVAL_ADDREF(p_);
    }

    ~value_ptr() {
        BOOST_PHP_ZVAL_ASSERT_REFCOUNT(p_);
        zval_ptr_dtor(reinterpret_cast<zval**>(&p_));
    }

    value_ptr& operator=(value_ptr const& rhs) {
        value_ptr(rhs).swap(*this);
        return *this;
    }

    void swap(value_ptr& that) {
        value* tmp( p_ );
        p_ = that.p_, that.p_ = tmp;
    }

    value* get() const {
        return p_;
    }

    value& operator*() {
        SEPARATE_ZVAL_IF_NOT_REF(reinterpret_cast<zval**>(&p_));
        return *p_;
    }

    const value& operator*() const {
        return *p_;
    }

    value* operator->() {
        SEPARATE_ZVAL_IF_NOT_REF(reinterpret_cast<zval**>(&p_));
        return p_;
    }

    const value* operator->() const {
        return p_;
    }

    operator const value*() const {
        return p_;
    }

    operator bool() const {
        return p_;
    }

    bool operator!() const {
        return !p_;
    }

    template<enum value::typecode TYPE_> value_ptr const as() const;

    template<enum value::typecode TYPE_> value_ptr as();
protected:
    value* p_;
};

} } // namespace boost::php 

#endif /* BOOST_PHP_VALUE_PTR_DEFINED */

#include <boost/php/exceptions.hpp>
#include <boost/php/hashtable.hpp>

#ifndef BOOST_PHP_VALUE_MEMBERS_DEFINED
#define BOOST_PHP_VALUE_MEMBERS_DEFINED

namespace boost { namespace php {

typedef hashtable<value_ptr, symtable_key> array;

inline value::value(value_ptr const& that): ::zval(*that) {
    zval_copy_ctor(static_cast<zval*>(this));
    initialize();
}

inline value::value(array const& that) {
    type = _ARRAY;
    ::zval::value.ht = const_cast<array*>(&that);
    zval_copy_ctor(this);
    initialize();
}

inline value::value(array* that) {
    type = _ARRAY;
    ::zval::value.ht = that;
    initialize();
}

inline value::value(::zval const& that, enum typecode desired_type): ::zval(that) {
    zval_copy_ctor(static_cast<zval*>(this));
    if (that.type != desired_type) {
        convert_to_explicit_type(this, desired_type);
        if (type != desired_type) {
            zval_dtor(this);
            throw type_error(
                    ::std::string("could not convert ")
                    + ::std::string(get_type_string(
                            static_cast<enum typecode>(that.type)))
                    + " to "
                    + ::std::string(get_type_string(desired_type)));
        }
    }
    initialize();
}

inline value::value(int v) {
    if (v < ::std::numeric_limits<long>::min()
            || v > ::std::numeric_limits<long>::max()) {
        throw std::overflow_error(
                ::std::string("cannot cast ")
                + ::boost::lexical_cast< ::std::string>(v) + "to long");
    }
    ::zval::type = IS_LONG;
    ::zval::value.lval = v;
    initialize();
}

inline const value value::concat(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == concat_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_equal(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_equal_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_not_equal(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_not_equal_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline value& value::increment() {
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == increment_function(this)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return *this;
}

inline value& value::decrement() {
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == decrement_function(this)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return *this;
}

inline const value value::add(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == add_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::sub(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == sub_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::mul(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == mul_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::div(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == div_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::mod(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == mod_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::bitwise_or(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == bitwise_or_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::bitwise_and(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == bitwise_and_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::bitwise_xor(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == bitwise_xor_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::shift_left(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == shift_left_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::shift_right(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == shift_right_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_smaller(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_smaller_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_greater_or_equal(::zval const& rhs TSRMLS_DC) const {
    return is_smaller(rhs TSRMLS_CC).boolean_not(TSRMLS_C);
}

inline const value value::is_smaller_or_equal(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_smaller_or_equal_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_greater(::zval const& rhs TSRMLS_DC) const {
    return is_smaller_or_equal(rhs TSRMLS_CC).boolean_not(TSRMLS_C);
}

inline const value value::is_identical(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_identical_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::is_not_identical(::zval const& rhs TSRMLS_DC) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == is_not_identical_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this)),
                const_cast<zval*>(&rhs) TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::boolean_or(::zval const& rhs TSRMLS_DC) const {
    return value(static_cast<bool>(zval_is_true(
            reinterpret_cast<zval*>(const_cast<value*>(this))) ||
        zval_is_true(const_cast<zval*>(&rhs))));
}

inline const value value::boolean_and(::zval const& rhs TSRMLS_DC) const {
    return value(static_cast<bool>(zval_is_true(
            reinterpret_cast<zval*>(const_cast<value*>(this))) &&
        zval_is_true(const_cast<zval*>(&rhs))));
}

inline const value value::boolean_not(TSRMLS_D) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == boolean_not_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this))
                TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline const value value::bitwise_not(TSRMLS_D) const {
    value retval;
    BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == bitwise_not_function(&retval,
                reinterpret_cast<zval*>(const_cast<value*>(this))
                TSRMLS_CC)) {
            throw arithmetic_error(BOOST_PHP_LAST_ERROR);
        }
    BOOST_PHP_END_CAPTURE_ERROR
    return retval;
}

inline value::operator string&() {
    if (::zval::type != _STRING) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to boost::php::string");
    }
    return *reinterpret_cast<string*>(&::zval::value.str);
}

inline value::operator string const&() const {
    if (::zval::type != _STRING) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to boost::php::string");
    }
    return *reinterpret_cast<const string*>(&::zval::value.str);
}

inline value::operator ::std::string() const
{
    if (::zval::type == _STRING) {
        return ::std::string(::zval::value.str.val, ::zval::value.str.len);
    }
    value tmp(*this, _STRING);
    return tmp;
}

inline value::operator array&()
{
    if (::zval::type != _ARRAY) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to hashtable<value_ptr>");
    }
    return *reinterpret_cast<array* >(::zval::value.ht);
}

inline value::operator array const&() const
{
    if (::zval::type != _ARRAY) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to hashtable<value_ptr>");
    }
    return *reinterpret_cast<array* >(::zval::value.ht);
}

inline value::operator resource_handle() const
{
    if (::zval::type != _RESOURCE) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to resource");
    }
    return resource_handle(::zval::value.lval);
}

inline value::operator long&() {
    if (::zval::type != _LONG) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to long");
    }
    return ::zval::value.lval;
}

inline value::operator long() const {
    if (::zval::type != _LONG) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to long");
    }
    return ::zval::value.lval;
}

inline value::operator double&() {
    if (::zval::type != _DOUBLE) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to double");
    }
    return ::zval::value.dval;
}

inline value::operator double const&() const {
    if (::zval::type != _DOUBLE) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to double");
    }
    return ::zval::value.dval;
}

inline value::operator ::zend_object_value const&() const
{
    if (::zval::type != _OBJECT) {
        throw type_error(
                ::std::string(get_type_string(::zval::type))
                + " value cannot be cast to object");
    }
    return ::zval::value.obj;
}

} } // namespace boost::php

#endif /* BOOST_PHP_VALUE_MEMBERS_DEFINED */

#ifndef BOOST_PHP_VALUE_PTR_MEMBER_DEFINED
#define BOOST_PHP_VALUE_PTR_MEMBER_DEFINED

namespace boost { namespace php {

template<enum value::typecode TYPE_>
inline value_ptr const value_ptr::as() const
{
    if (static_cast<enum value::typecode>(p_->type) == TYPE_) {
        return *this;
    }
    return value_ptr(new value(*p_, TYPE_), false);
}

template<enum value::typecode TYPE_>
inline value_ptr value_ptr::as()
{
    if (static_cast<enum value::typecode>(p_->type) != TYPE_) {
        throw type_error(::std::string("write access to ")
                + value::get_type_string(p_->type)
                + " as " + value::get_type_string(TYPE_)
                + " requested");
    }
    return *this;
}

} } // namespace boost::php

#endif /* BOOST_PHP_VALUE_PTR_MEMBER_DEFINED */

#include <boost/php/converter.hpp>
#include <boost/php/detail/stream_support.hpp>
