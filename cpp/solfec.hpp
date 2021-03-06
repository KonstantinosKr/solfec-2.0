/*
The MIT License (MIT)

Copyright (c) 2019 EDF Energy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>

#ifndef __solfec__
#define __solfec__

/* spline */
struct spline
{
  std::vector<std::array<REAL,2>> points; /* vector of (x, y) pairs */
  uint64_t cache; /* partial cache size */
  std::string path; /* file path for partially cached splines */
  std::vector<off_t> offset; /* file offsets */
  std::vector<REAL> xval; /* x values maching file offsets */
  uint64_t marker; /* index of the last read interval */
  spline(): marker(0), cache(0) {}
};

/* material */
struct material
{
  REAL density; /* material density */
  REAL young; /* Young's modulus */
  REAL poisson; /* Poisson's ratio */
  REAL viscosity; /* viscosisty coefficient */
};

/* body */
struct body
{
  uint64_t matnum; /* material number */
  uint64_t gcolor; /* global color */
  body () : matnum(0), gcolor(0) { }
};

/* mesh */
struct mesh: public body
{
  std::array<std::vector<REAL>,3> nodes; /* list of nodes */
  std::vector<uint64_t> elements; /* list of elements */
  std::vector<uint64_t> colors; /* face colors */
  uint64_t nhex, nwed, npyr, ntet; /* numbers of elements per kind */
  uint64_t nfaces; /* number of faces */
  mesh () : nhex(0), nwed(0), npyr(0), ntet(0), nfaces(0) { }
};

/* ellipsoid */
struct ellip: public body
{
  REAL center[3];
  REAL radius[3];
  REAL rotation[9];
  ellip () : rotation{1,0,0,0,1,0,0,0,1} { }
};

/* restrain */
struct restrain
{
  uint64_t bodnum; /* body number */
  std::vector<std::array<REAL,3>> points; /* list of points */
  uint64_t color; /* surface color */
  REAL direction[3]; /* restrain direction */
  restrain() : bodnum(0), color(0), direction{0.,0.,0.} { }
};

/* prescribe */
struct prescribe
{
  uint64_t bodnum; /* body number */
  std::vector<std::array<REAL,3>> points; /* list of points */
  uint64_t color; /* surface color */
  bool linear_applied; /* linear applied flag */
  REAL linear_values [3]; /* constant linear velocity */
  int64_t linear_splines [3]; /* linear splines (when >= 0) */
  void* linear_callback; /* Python callback function */
  bool angular_applied;
  REAL angular_values [3];
  int64_t angular_splines [3];
  void* angular_callback;
  prescribe() : bodnum(0), color(0), linear_applied(false), angular_applied(false) { }
};

/* velocity */
struct velocity
{
  REAL linear_values [3]; /* linear velocity */
  REAL angular_values [3]; /* linear velocity */
};

/* friction */
struct friction
{
  uint64_t color1; /* first face color */
  uint64_t color2; /* second face color */
  REAL static_friction; /* static Coulomb's friction */
  REAL dynamic_friction;/* dynamic Coulomb's friction */
};

/* friction comparison */
struct friction_compare {
  bool operator() (const struct friction& lhs, const struct friction& rhs) const
  {return lhs.color1 == rhs.color1 ? lhs.color2 < rhs.color2 : lhs.color1 < lhs.color2;}
};

/* gravity */
struct gravity
{
  REAL gvalue [3]; /* constant values */
  int64_t gspline [3]; /* linear splines (when >= 0) */
  void* gcallback [3]; /* Python callback functions */
  gravity() : gvalue{0,0,0}, gspline{-1,-1,-1}, gcallback{NULL,NULL,NULL} {}
  void clear() {gvalue[0]=gvalue[1],gvalue[2]=0.;gspline[0]=gspline[1]=gspline[1]=-1;gcallback[0]=gcallback[1]=gcallback[2]=NULL;}
};

/* history */
struct history
{
  std::string entity; /* entity name */
  std::vector<std::array<REAL,3>> points; /* list of points */
  uint64_t bodnum; /* body number */
  std::string filepath; /* text file path */
  void *python_list; /* python list */
  history() : python_list(NULL) { }
};

/* output */
struct output
{
  std::set<std::string> entities; /* output entities */
  std::set<uint64_t> subset; /* subset of body numbers */
  std::set<std::string> modes; /* output modes */
};

/* Python callback function */
typedef void* pointer_t;

/* interval */
struct interval
{
  pointer_t dt_function[2];
  int64_t dt_spline[2];
  REAL dt[2];
};

/* solfec global variables */
namespace solfec
{
extern int argc;
extern char **argv;
extern std::string outname;
extern REAL simulation_time;
extern std::map<uint64_t,spline> splines;
extern uint64_t splines_count;
extern std::map<uint64_t,material> materials;
extern uint64_t materials_count;
extern std::map<uint64_t,mesh> meshes;
extern std::map<uint64_t,ellip> ellips;
extern uint64_t bodies_count;
extern std::map<uint64_t,std::set<uint64_t>> body_restrains;
extern std::map<uint64_t,restrain> restrains;
extern uint64_t restrains_count;
extern std::map<uint64_t,std::set<uint64_t>> body_prescribes;
extern std::map<uint64_t,prescribe> prescribes;
extern uint64_t prescribes_count;
extern std::map<uint64_t,std::vector<velocity>> velocities;
extern std::set<friction,friction_compare> frictions;
extern struct gravity gravity;
extern std::vector<history> histories;
extern std::vector<output> outputs;
extern struct interval interval;
};

/* read spline from file */
void spline_from_file (const char *path, int cache, struct spline *spline);

/* calculate splane value */
REAL spline_value (struct spline *spline, REAL xval);

/* interpret an input file (return 0 on success) */
int input_python (const char *path);

/* output current results */
void output_current_results ();

#endif
