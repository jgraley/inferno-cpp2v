/*
 * generate_implicit_casts.hpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_IMPLICIT_CASTS_HPP
#define GENERATE_IMPLICIT_CASTS_HPP

#include "helpers/search_replace.hpp"

class GenerateImplicitCasts
{
public:
	virtual void operator()( shared_ptr<Program> );
};

#endif /* GENERATE_IMPLICIT_CASTS_HPP */
