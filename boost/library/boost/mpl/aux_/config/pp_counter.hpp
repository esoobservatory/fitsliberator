
#ifndef BOOST_MPL_AUX_CONFIG_PP_COUNTER_HPP_INCLUDED
#define BOOST_MPL_AUX_CONFIG_PP_COUNTER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2006
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /project24/CVS/liberator/boost/library/boost/mpl/aux_/config/pp_counter.hpp,v $
// $Date: 2008/04/19 09:38:41 $
// $Revision: 1.1 $

#if !defined(BOOST_MPL_AUX_PP_COUNTER)
#   include <boost/mpl/aux_/config/msvc.hpp>
#   if BOOST_WORKAROUND(BOOST_MSVC, >= 1300)
#       define BOOST_MPL_AUX_PP_COUNTER() __COUNTER__
#   else
#       define BOOST_MPL_AUX_PP_COUNTER() __LINE__
#   endif
#endif

#endif // BOOST_MPL_AUX_CONFIG_PP_COUNTER_HPP_INCLUDED
