/*
 * generate_implicit_casts.hpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_IMPLICIT_CASTS_HPP
#define GENERATE_IMPLICIT_CASTS_HPP

#include "helpers/search_replace.hpp"

// Wherever an argument to a function call differs in type from
// the funciton's declaration, insert a cast to the declared
// type.
class GenerateImplicitCasts : public SearchReplace
{
public:
	GenerateImplicitCasts();
};

#endif /* GENERATE_IMPLICIT_CASTS_HPP */
