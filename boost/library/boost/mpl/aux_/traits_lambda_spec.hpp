
#ifndef BOOST_MPL_AUX_TRAITS_LAMBDA_SPEC_HPP_INCLUDED
#define BOOST_MPL_AUX_TRAITS_LAMBDA_SPEC_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /project24/CVS/liberator/boost/library/boost/mpl/aux_/traits_lambda_spec.hpp,v $
// $Date: 2008/04/19 09:38:41 $
// $Revision: 1.4 $

#include <boost/mpl/void.hpp>
#include <boost/mpl/aux_/preprocessor/params.hpp>
#include <boost/mpl/aux_/config/lambda.hpp>

#if !defined(BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)

#   define BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(i, trait) /**/

#elif !defined(BOOST_MPL_CFG_MSVC_ETI_BUG)

#   define BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(i, trait) \
template<> struct trait<void_> \
{ \
    template< BOOST_MPL_PP_PARAMS(i, typename T) > struct apply \
    { \
    }; \
}; \
/**/

#else

#   define BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(i, trait) \
template<> struct trait<void_> \
{ \
    template< BOOST_MPL_PP_PARAMS(i, typename T) > struct apply \
    { \
    }; \
}; \
template<> struct trait<int> \
{ \
    template< BOOST_MPL_PP_PARAMS(i, typename T) > struct apply \
    { \
        typedef int type; \
    }; \
}; \
/**/

#endif // BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#endif // BOOST_MPL_AUX_TRAITS_LAMBDA_SPEC_HPP_INCLUDED
