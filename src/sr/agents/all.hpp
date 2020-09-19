
// Legacy clients of SR have assumed that all these includes come from
// search_replace.hpp. This would not be so under the agents scheme. 
// We compromise by putting them here. Putting them after the code clarifies
// that there is no dependency; just the will to be nice to client code.
#include "standard_agent.hpp"
#include "search_container_agent.hpp"
#include "star_agent.hpp"
#include "green_grass_agent.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "match_all_agent.hpp"
#include "match_any_agent.hpp"
#include "not_match_agent.hpp"
#include "pointer_is_agent.hpp"
#include "transform_of_agent.hpp"
