#ifndef ephemerid_hpp__
#define ephemerid_hpp__

#include <calceph/calceph.h>

// CALCEPH target numbers for calceph_compute (no CALCEPH_USE_NAIFID flag).
// See https://calceph.imcce.fr/docs/5.0.1/calceph_c.pdf
// Note: in DE/INPOP ephemerides Mars..Neptune resolve to the planet
// barycenter, not the planet center.
enum class celestial_body {
  Mercury = 1,
  Venus = 2,
  Earth = 3,
  Mars = 4,
  Jupiter = 5,
  Saturn = 6,
  Uranus = 7,
  Neptune = 8,
  Moon = 10,
  Sun = 11
};

class Ephemerid {

public:
};

#endif
