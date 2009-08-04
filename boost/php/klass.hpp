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

#ifndef BOOST_PHP_KLASS_DEFINED
#define BOOST_PHP_KLASS_DEFINED
#include <new>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <zend_API.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/php/detail/function_template.hpp>
#include <boost/php/detail/function_container.hpp>
#include <boost/php/detail/object_retriever.hpp>
#include <boost/php/object.hpp>
#include <boost/php/exceptions.hpp>

namespace boost { namespace php {

template<typename T_, typename Tnoncopyable_>
class klass;

template<typename T_>
struct klass_registry
{
    static klass<T_, void>* value;
};

template<typename T_>
klass<T_, void>* klass_registry<T_>::value(0);

struct klass_base {
protected:
    template<typename T_>
    class zo_wrapper_base: public sensible_object {
    public:
        typedef T_ object_type;
        typedef boost::shared_ptr<object_type> shared_pointer;

        zo_wrapper_base(zend_class_entry const& ce TSRMLS_DC)
            : sensible_object(ce TSRMLS_CC),
              type(INSTANCE), ptr(reinterpret_cast<object_type*>(instance)),
              destroy_handler(&instance_destructor) {}

        zo_wrapper_base(zend_class_entry const& ce, object_type* _ptr,
                void(*_destroy_handler)(object_type* TSRMLS_DC) = noop_destructor
                TSRMLS_DC)
            : sensible_object(ce TSRMLS_CC), type(WEAKREF), ptr(_ptr),
              destroy_handler(_destroy_handler) {}

        zo_wrapper_base(zend_class_entry const& ce,
                shared_pointer const& _ptr TSRMLS_DC)
            : sensible_object(ce TSRMLS_CC), type(REF),
              ptr((new (ref) shared_pointer(_ptr))->get()) {}

    private:    
        static void instance_destructor(object_type* ptr TSRMLS_DC)
        {
            ptr->~object_type();
        }

        static void noop_destructor(object_type* ptr TSRMLS_DC)
        {
        }

    public:
        enum { WEAKREF, INSTANCE, REF } type:2;

        object_type* ptr;
        void (*destroy_handler)(object_type* TSRMLS_DC);

        union {
            unsigned char instance[sizeof(object_type)];
            unsigned char ref[sizeof(shared_pointer)];
        };
    };

    template<typename T_, typename Tnoncopyable_>
    struct zo_wrapper_tpl: public zo_wrapper_base<T_> {
        typedef T_ object_type;
        typedef zo_wrapper_base<T_> base_type;
        typedef typename base_type::shared_pointer shared_pointer;

        zo_wrapper_tpl(zend_class_entry const& ce TSRMLS_DC)
            : base_type(ce TSRMLS_CC) {}

        zo_wrapper_tpl(zo_wrapper_tpl const& that TSRMLS_DC)
            : base_type(*that.ce TSRMLS_CC) {
            this->type = that.type;
            this->destroy_handler = that.destroy_handler;
            switch (that.type) {
            case base_type::WEAKREF:
                this->ptr = that.ptr;
                break;
            case base_type::INSTANCE:
                this->ptr = new (this->instance) object_type(
                    *reinterpret_cast<object_type const*>(that.instance));
                break;
            case base_type::REF:
                this->ptr = (new (this->ref) shared_pointer(
                    *reinterpret_cast<shared_pointer const*>(that.ref)))->get();
                break;
            }
        }

        zo_wrapper_tpl(zend_class_entry const& ce, object_type* ptr,
                void(*destroy_handler)(object_type* TSRMLS_DC) = 0 TSRMLS_DC)
            : base_type(ce, ptr, destroy_handler TSRMLS_CC) {}

        zo_wrapper_tpl(zend_class_entry const& ce,
                shared_pointer ptr TSRMLS_DC)
            : base_type(ce, ptr TSRMLS_CC) {}
    };
};

template<typename T_>
struct klass_base::zo_wrapper_tpl<T_, boost::noncopyable> {
    typedef T_ object_type;
    typedef zo_wrapper_base<T_> base_type;
    typedef typename base_type::shared_pointer shared_pointer;

    zo_wrapper_tpl(zend_class_entry const& ce TSRMLS_DC)
        : base_type(ce TSRMLS_CC) {}

    zo_wrapper_tpl(zo_wrapper_tpl const& that TSRMLS_DC)
        : base_type(*that.ce TSRMLS_CC) {
        this->type = that.type;
        this->destroy_handler = that.destroy_handler;
        switch (that.type) {
        case base_type::WEAKREF:
            this->ptr = that.ptr;
            break;
        case base_type::INSTANCE:
            throw ::std::runtime_error(
                "underlying object is not copy-constructible");
            break;
        case base_type::REF:
            this->ptr = (new (this->ref) shared_pointer(
                *reinterpret_cast<shared_pointer const*>(that.ref)))->get();
            break;
        }
    }

    zo_wrapper_tpl(zend_class_entry const& ce, object_type* ptr,
            void(*destroy_handler)(object_type* TSRMLS_DC) = 0 TSRMLS_DC)
        : zo_wrapper_base<T_>(ce, ptr, destroy_handler TSRMLS_CC) {}

    zo_wrapper_tpl(zend_class_entry const& ce,
            shared_pointer ptr TSRMLS_DC)
        : base_type(ce, ptr TSRMLS_CC) {}
};

template<typename T_, typename Tnoncopyable_ = void>
class klass: public ::zend_class_entry, public function_container<klass<T_> >, public klass_base {
    friend class object_retriever<T_>;
    friend class to_native_converter<T_, false>;
    friend struct to_value_ptr_converter<T_*, false>;
    friend struct to_value_ptr_converter<T_&, false>;
public:
    typedef T_ object_type;
    typedef zo_wrapper_tpl<object_type, Tnoncopyable_> zo_wrapper;
    friend struct to_value_ptr_converter<typename zo_wrapper::shared_pointer, false>;

private:
    typedef boost::unordered_map<object_type*, ::zend_object_handle> instance_handle_map;

public:
    klass(char const* _name) {
        TSRMLS_FETCH();
        type = ZEND_INTERNAL_CLASS;
        zend_initialize_class_data(this, true TSRMLS_CC);
        ce_flags = 0;
        name_length = strlen(_name);
        name = static_cast<char*>(::std::malloc(name_length + 1));
        if (!name)
            throw ::std::bad_alloc();
        ::std::memcpy(name, _name, name_length + 1);
        create_object = reinterpret_cast<
            ::zend_object_value(*)(::zend_class_entry* TSRMLS_DC)>(&__factory);
    }

    template<typename Tsig_>
    function_entry& define_function(
            ::std::string const& name, Tsig_ const& sig) {
        function_entry& retval =
            function_container<klass>::define_function(name, sig);
        retval.flags |= ZEND_ACC_PUBLIC;
        builtin_functions = *this;
        return retval;
    }

    template<typename Targs_>
    klass& ctor(Targs_) {
        typedef detail::unbound_function<detail::constructor_mark, T_, Targs_>
                ctor_type;
        define_function(ZEND_CONSTRUCTOR_FUNC_NAME,
                detail::signature<ctor_type>(ctor_type()));
        constructor = &(
                *(--reinterpret_cast<hashtable< ::zend_function>&>(
                        function_table).end())).second;
        return *this;
    }

    void fixup() {
        TSRMLS_FETCH();
        BOOST_PHP_BEGIN_CAPTURE_ERROR
        if (FAILURE == ::zend_register_functions(
                this, *this,
                &function_table, MODULE_PERSISTENT TSRMLS_CC)) {
            throw runtime_error(BOOST_PHP_LAST_ERROR);
        }
        BOOST_PHP_END_CAPTURE_ERROR
    }

    static void* operator new(std::size_t sz) {
        void* retval = ::std::malloc(sz);
        if (!retval)
            throw ::std::bad_alloc();
        return retval;
    }

    static void operator delete(void* ptr) {
        ::std::free(ptr);
    }

private:
    static ::zend_object_value __factory(klass* self TSRMLS_DC) {
        zend_object_value retval;

        zo_wrapper* obj = new zo_wrapper(*self TSRMLS_CC);
        obj->handle = retval.handle = ::zend_objects_store_put(obj,
                reinterpret_cast< ::zend_objects_store_dtor_t>(&__dtor_wrapper),
                reinterpret_cast< ::zend_objects_free_object_storage_t>(
                    &__delete_wrapper),
                reinterpret_cast< ::zend_objects_store_clone_t>(
                    &__cctor_wrapper)
                TSRMLS_CC);
        retval.handlers = ::zend_get_std_object_handlers();
        return retval;
    }

    static void __deleter(object_type* ptr TSRMLS_DC)
    {
        delete ptr;
    }

    static ::zend_object_value __weakref_factory(klass* self, object_type* ptr, void(*destroy_handler)(object_type* TSRMLS_DC) = 0 TSRMLS_DC) {
        zend_object_value retval;

        typename instance_handle_map::const_iterator i(weakref_handle_map_.find(ptr));
        if (weakref_handle_map_.end() != i) {
            retval.handle = (*i).second;
            zend_objects_store_add_ref_by_handle(retval.handle TSRMLS_CC);
        } else {
            zo_wrapper* obj = new zo_wrapper(*self, ptr, destroy_handler TSRMLS_CC);
            retval.handle = obj->handle = ::zend_objects_store_put(obj,
                    reinterpret_cast< ::zend_objects_store_dtor_t>(
                        &__dtor_wrapper),
                    reinterpret_cast< ::zend_objects_free_object_storage_t>(
                        &__delete_wrapper),
                    reinterpret_cast< ::zend_objects_store_clone_t>(
                        &__cctor_wrapper)
                    TSRMLS_CC);
            weakref_handle_map_.insert(std::make_pair(ptr, retval.handle));
        }
        retval.handlers = ::zend_get_std_object_handlers();
        return retval;
    }

    static ::zend_object_value __ref_factory(klass* self, typename zo_wrapper::shared_pointer ptr TSRMLS_DC) {
        zend_object_value retval;

        typename instance_handle_map::const_iterator i(ref_handle_map_.find(ptr.get()));
        if (ref_handle_map_.end() != i) {
            retval.handle = (*i).second;
            zend_objects_store_add_ref_by_handle(retval.handle TSRMLS_CC);
        } else {
            zo_wrapper* obj = new zo_wrapper(*self, ptr TSRMLS_CC);
            retval.handle = obj->handle = ::zend_objects_store_put(obj,
                    reinterpret_cast< ::zend_objects_store_dtor_t>(
                        &__dtor_wrapper),
                    reinterpret_cast< ::zend_objects_free_object_storage_t>(
                        &__delete_wrapper),
                    reinterpret_cast< ::zend_objects_store_clone_t>(
                        &__cctor_wrapper)
                    TSRMLS_CC);
            ref_handle_map_.insert(std::make_pair(ptr.get(), retval.handle));
        }
        retval.handlers = ::zend_get_std_object_handlers();
        return retval;
    }


    static void __dtor_wrapper(zo_wrapper* obj TSRMLS_DC) {
        if (obj->type == zo_wrapper::REF) {
            ref_handle_map_.erase(obj->ptr);
            typedef typename zo_wrapper::shared_pointer shared_pointer;
            reinterpret_cast<shared_pointer*>(obj->ref)->~shared_pointer();
        } else {
            if (obj->type == zo_wrapper::WEAKREF)
                weakref_handle_map_.erase(obj->ptr);
            if (obj->destroy_handler)
                obj->destroy_handler(obj->ptr TSRMLS_CC);  
        }
    }

    static void __delete_wrapper(zo_wrapper* obj TSRMLS_DC) {
        delete obj;
    }

    static void __cctor_wrapper(zo_wrapper* obj, zo_wrapper** cloned_obj TSRMLS_DC) {
        *cloned_obj = new zo_wrapper(*obj TSRMLS_CC);
    }

    static instance_handle_map weakref_handle_map_;
    static instance_handle_map ref_handle_map_;
};

template<typename T_, typename Tnoncopyable_>
typename klass<T_, Tnoncopyable_>::instance_handle_map
klass<T_, Tnoncopyable_>::weakref_handle_map_;

template<typename T_, typename Tnoncopyable_>
typename klass<T_, Tnoncopyable_>::instance_handle_map
klass<T_, Tnoncopyable_>::ref_handle_map_;

template<typename T_>
T_* object_retriever<T_>::operator()(INTERNAL_FUNCTION_PARAMETERS) const
{
    typedef typename klass<T_>::zo_wrapper wrapper_type;
    wrapper_type* w = reinterpret_cast<wrapper_type*>(
            ::zend_objects_get_address(this_ptr TSRMLS_CC));
    return w->ptr;
}

template<typename T_, typename Tctor_args_>
static klass<T_>& def_class(char const* name, Tctor_args_ args TSRMLS_DC)
{
    klass<T_>* retval = new klass<T_>(name);
    boost::scoped_array<char> lowercased_name(new char[retval->name_length + 1]);
    retval->module = EG(current_module);
    retval->ctor(args);
    ::zend_str_tolower_copy(lowercased_name.get(), retval->name, retval->name_length);
    ::zend_hash_update(CG(class_table), lowercased_name.get(), retval->name_length + 1,
            &retval, sizeof(zend_class_entry*), NULL);
    klass_registry<T_>::value = retval;
    return *retval;
}

template<typename T_>
static klass<T_>& def_class(char const* name TSRMLS_DC)
{
    return def_class<T_, ::boost::mpl::vector0<> >(name, ::boost::mpl::vector0<>() TSRMLS_CC);
}

} } // namespace boost::php

template<typename T_>
void intrusive_ptr_add_ref(::boost::php::klass<T_>* ptr)
{
    ++ptr->refcount;
}

template<typename T_>
void intrusive_ptr_release(::boost::php::klass<T_>* ptr)
{
    ::destroy_zend_class(&ptr);
}
#endif /* BOOST_PHP_KLASS_DEFINED */

#ifndef BOOST_PHP_KLASS_STD_CONVERTER_DEFINED
#define BOOST_PHP_KLASS_STD_CONVERTER_DEFINED
#include <boost/php/converter.hpp>

namespace boost { namespace php {

template<typename T_>
struct to_native_converter<T_, false> {
    T_ const& operator()(value_ptr const& val TSRMLS_DC) const {
        return (*this)(const_cast<value_ptr&>(val));
    }

    T_& operator()(value_ptr& val TSRMLS_DC) const {
        return *reinterpret_cast<typename klass<T_>::zo_wrapper*>(
                    zend_objects_get_address(
                        const_cast< ::zval*>(
                            static_cast< ::zval const*>(
                                val.as<value::_OBJECT>()))
                        TSRMLS_CC))->ptr;
    }
};

template<typename T_>
struct to_value_ptr_converter<T_*, false> {
    value_ptr operator()(T_* val TSRMLS_DC) const {
        return value_ptr(new value(klass<T_>::__weakref_factory(klass_registry<T_>::value, val, klass<T_>::__deleter)), false);
    }
};

template<typename T_>
struct to_value_ptr_converter<T_ const*, false> {
    value_ptr operator()(T_ const* val TSRMLS_DC) const {
        return to_value_ptr_converter<T_*, false>()(const_cast<T_*>(val) TSRMLS_CC);
    }
};

template<typename T_>
struct to_value_ptr_converter<T_&, false> {
    value_ptr operator()(T_& val TSRMLS_DC) const {
        return value_ptr(new value(klass<T_>::__weakref_factory(klass_registry<T_>::value, &val)), false);
    }
};

template<typename T_>
struct to_value_ptr_converter<T_ const&, false> {
    value_ptr operator()(T_ const& val TSRMLS_DC) const {
        return to_value_ptr_converter<T_&, false>()(const_cast<T_&>(val) TSRMLS_CC);
    }
};

template<typename T_>
struct to_value_ptr_converter<boost::shared_ptr<T_>, false> {
    value_ptr operator()(boost::shared_ptr<T_> const& val TSRMLS_DC) const {
        return value_ptr(new value(klass<T_>::__ref_factory(klass_registry<T_>::value, val)), false);
    }
};

} } // namespace boost::php
#endif /* BOOST_PHP_KLASS_STD_CONVERTER_DEFINED */
