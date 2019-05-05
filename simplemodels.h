
//----------------------------------------------------------------------------

const int NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

extern vec4 points_cube[NumVerticesCube];
extern vec4 colors[NumVerticesCube];
extern vec3 normals_cube[NumVerticesCube];
extern vec2 tex_coords_cube[NumVerticesCube];

void colorcube();


//----------------------------------------------------------------------------

const int segments = 64;
const int NumVerticesCylinder = segments*6 + segments*3*2;

extern vec4 points_cylinder[NumVerticesCylinder];
extern vec3 normals_cylinder[NumVerticesCylinder];
extern vec2 tex_coords_cylinder[NumVerticesCylinder];

void colortube();


//----------------------------------------------------------------------------

const int ssegments = 64;
const int NumVerticesSphere = (ssegments*6*(ssegments-2) + ssegments*3*2);//ssegments*6*(ssegments-2) + ssegments*3*2;

extern vec4 points_sphere[NumVerticesSphere];
extern vec3 normals_sphere[NumVerticesSphere];
extern vec2 tex_coords_sphere[NumVerticesSphere];

void colorbube();


//----------------------------------------------------------------------------
// hemisphere model

const int hemiseg = 64;
const int NumVerticesHemi = (hemiseg * 6 * (hemiseg)) * hemiseg;

extern vec4 points_hemi[NumVerticesHemi];
extern vec3 normals_hemi[NumVerticesHemi];
extern vec2 tex_coords_hemi[NumVerticesHemi];

void colorhemi();
