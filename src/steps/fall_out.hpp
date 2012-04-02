#ifndef FALL_OUT_HPP
#define FALL_OUT_HPP

#include "sr/search_replace.hpp"

namespace Steps {

/// PPut all the labels in an array indexable by an enum
class PlaceLabelsInArray : public SearchReplace
{
public:
    PlaceLabelsInArray();
};

}; // end namespace

#endif

