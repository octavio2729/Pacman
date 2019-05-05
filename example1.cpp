#if 1
// spherical camera + persp + lighting + texture
//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
using namespace std;

#include <GLUT/glut.h>

#include "Angel.h"
#include "simplemodels.h"

GLuint loadBMP_custom(unsigned char** data, int* w, int* h, const char * imagepath);


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Zaxis;
int frame = 0;
GLfloat  Theta1[NumAxes] = { 80.0, 0.0, 0.0 };
GLfloat  Theta2[NumAxes] = { 0.0, 30.0, 0.0 };
GLfloat  Theta3[NumAxes] = { 0.0, 0.0, 0.0 };
GLfloat  Theta4[NumAxes] = { 0.0, 0.0, 0.0 };
GLfloat  ThetaPac = 0.0; // this controls how wide pac mans mouth with open
GLfloat  someface = 0.0; // this controls the way the ghosts are facing, which will also be in the direction they are moving.
GLfloat  rotatebot = 0.0; // this rotates the bottom of the ghosts at a constant rate
GLfloat  pacX = 0.0; // I hope these names are self-explanatory enough...
GLfloat  pacZ = 18.0;
GLfloat  pacY = 1.0;
GLfloat  floatman = 0.0;
GLfloat  SpawnX = 0.0; // when you spawn a ghost, they will be set to this position and then have a direction assigned.
GLfloat  SpawnZ = -6.0;
GLfloat  BlinkyX = 0.0;
GLfloat  BlinkyZ = -6.0;
GLfloat  InkyX = -4.0;
GLfloat  InkyZ = 0.0;
GLfloat  PinkyX = 0.0;
GLfloat  PinkyZ = 0.0;
GLfloat  ClydeX = 4.0;
GLfloat  ClydeZ = 0.0;
GLboolean defaultmove = false; // this is default mode for the ghost AI
GLboolean chasemove = true; // this is the mode for chasing pacman
GLboolean scatter = false; // this is when they run away after pacman eats a power pellet.
GLboolean gamestart = false;
GLboolean block = false;
// these booleans say when we need to spawn the ghosts, and how we start their movements
GLboolean spawnBlinky = true; // (s)he starts out being spawned.
GLboolean spawnInky = false; // (s)he spawns almost immedietly after Blinky.
GLboolean spawnPinky = false; // (s)he spawns after some time.
GLboolean spawnClyde = false; // (s)he spawns after so many dots are collected.
GLboolean moveBlinky = false;
GLboolean movePinky = false;
GLboolean moveInky = false;
GLboolean moveClyde = false;
// these are just constants showing the directions based off their angles.
const int cardinalup = 180;
const int cardinalright = 90;
const int cardinaldown = 0;
const int cardinalleft = 270;
GLint PacD = 0; // these are the directions of each object Pacman, and the ghosts
GLint PinkyD = 0;
GLint ClydeD = 0;
GLint BlinkyD = 0;
GLint InkyD = 0;
GLfloat PacSpeed = 0.525; // and their speeds, the ghosts are barely slower
GLfloat GhostSpeed = 0.5;
double x = 0.0;
double y = 0.0;
double z = 0.0;
double yArray [237];
int globalNumber = 236;
int ballGrid [60][60];
int globalCounter = 0;
//This is all for testing purposes
int ballsEaten = 0;


//------------- RIGID BODIES -------------------------------------

//RigidBody toppac("toppac");
//RigidBody topmouf("topdisc");
//RigidBody botpac("botpac");
//RigidBody botmouf("botdisc");
//RigidBody wall2("wall2");
//RigidBody wall3("wall3");
//RigidBody wall4("wall4");

//vector<RigidBody> scene;

size_t CUBE_OFFSET;
size_t COLOR_OFFSET;
size_t CUBE_NORMALS_OFFSET;
size_t CUBE_TEXCOORDS_OFFSET;
size_t CYLINDER_OFFSET;
size_t CYLINDER_NORMALS_OFFSET;
size_t CYLINDER_TEXCOORDS_OFFSET;
size_t SPHERE_OFFSET;
size_t SPHERE_NORMALS_OFFSET;
size_t SPHERE_TEXCOORDS_OFFSET;
size_t HEMI_OFFSET;
size_t HEMI_NORMALS_OFFSET;
size_t HEMI_TEXCOORDS_OFFSET;


//----------------------------------------------------------------------------

GLuint program;

GLuint vPosition;
GLuint vNormal;
GLuint vColor;
GLuint vTexCoord;
//GLuint vTexCoordCyl;


GLuint textures[22];

void
init()
{
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader21.glsl", "fshader21.glsl" );
    glUseProgram( program );
    
    //---------------------------------------------------------------------
    colorcube();
    colortube();
    colorbube();
    colorhemi();
    //---------------------------------------------------------------------
    
    //----- generate a checkerboard pattern for texture mapping
    const int  TextureSize  = 256;
    GLubyte checker_tex[TextureSize][TextureSize][3];
    
    for ( int i = 0; i < TextureSize; i++ )
    {
        for ( int j = 0; j < TextureSize; j++ )
        {
            //GLubyte c = (((i & 0x4) == 0) ^ ((j & 0x1)  == 0)) * 255;
            GLubyte c = (((j & 0x2)  == 0)) * 255;
            checker_tex[i][j][0]  = c;
            checker_tex[i][j][1]  = c;
            checker_tex[i][j][2]  = c;
        }
    }
    
    //---- Initialize texture objects
    glGenTextures( 22, textures );
    
    glActiveTexture( GL_TEXTURE0 );
    
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0, GL_BGR, GL_UNSIGNED_BYTE, checker_tex );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic1 = NULL;
    int w,h;
    loadBMP_custom(&pic1, &w, &h, "pacmanmap.bmp"); //formerly cilo.bmp
    
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Cyan.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "CyanFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[3] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "CyanLower.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[4] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "InsidePacman.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[5] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Orange.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[6] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "OrangeFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[7] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "OrangeLower.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[8] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "pacman.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[9] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Pink.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[10] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "PinkFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[11] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "PinkLower.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[12] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Red.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[13] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "RedFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[14] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Red.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[15] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "RedFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[16] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    loadBMP_custom(&pic1, &w, &h, "RedLower.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[17] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Yellow.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[18] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "Blue.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[19] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "BlueFace.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[20] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    loadBMP_custom(&pic1, &w, &h, "blueghost.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[21] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    
    //----set offset variables
    
    CUBE_OFFSET = 0;
    COLOR_OFFSET = sizeof(points_cube);
    CUBE_NORMALS_OFFSET = COLOR_OFFSET + sizeof(colors);
    CUBE_TEXCOORDS_OFFSET = CUBE_NORMALS_OFFSET + sizeof(normals_cube);
    CYLINDER_OFFSET = CUBE_TEXCOORDS_OFFSET + sizeof(tex_coords_cube);
    CYLINDER_NORMALS_OFFSET = CYLINDER_OFFSET + sizeof(points_cylinder);
    CYLINDER_TEXCOORDS_OFFSET = CYLINDER_NORMALS_OFFSET + sizeof(normals_cylinder);
    SPHERE_OFFSET = CYLINDER_TEXCOORDS_OFFSET + sizeof(tex_coords_cylinder);
    SPHERE_NORMALS_OFFSET = SPHERE_OFFSET + sizeof(points_sphere);
    SPHERE_TEXCOORDS_OFFSET = SPHERE_NORMALS_OFFSET + sizeof(normals_sphere);
    HEMI_OFFSET = SPHERE_TEXCOORDS_OFFSET + sizeof(tex_coords_sphere);
    HEMI_NORMALS_OFFSET = HEMI_OFFSET + sizeof(points_hemi);
    HEMI_TEXCOORDS_OFFSET = HEMI_NORMALS_OFFSET + sizeof(normals_hemi);
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );  // removed 'APPLE' suffix for 3.2
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(normals_cube) + sizeof(tex_coords_cube)
                 + sizeof(colors) + sizeof(points_cylinder) + sizeof(normals_cylinder)+ sizeof(tex_coords_cylinder)
                 + sizeof(points_sphere) + sizeof(normals_sphere) + sizeof(tex_coords_sphere)
                 + sizeof(points_hemi) + sizeof(normals_hemi) + sizeof(tex_coords_hemi), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_OFFSET, sizeof(points_cube), points_cube );
    glBufferSubData( GL_ARRAY_BUFFER, COLOR_OFFSET, sizeof(colors), colors );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_NORMALS_OFFSET, sizeof(normals_cube), normals_cube );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_TEXCOORDS_OFFSET, sizeof(tex_coords_cube), tex_coords_cube );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_OFFSET, sizeof(points_cylinder), points_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_NORMALS_OFFSET, sizeof(normals_cylinder), normals_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_TEXCOORDS_OFFSET, sizeof(tex_coords_cylinder), tex_coords_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_OFFSET, sizeof(points_sphere), points_sphere );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_NORMALS_OFFSET, sizeof(normals_sphere), normals_sphere );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_TEXCOORDS_OFFSET, sizeof(tex_coords_sphere), tex_coords_sphere );
    glBufferSubData( GL_ARRAY_BUFFER, HEMI_OFFSET, sizeof(points_hemi), points_hemi );
    glBufferSubData( GL_ARRAY_BUFFER, HEMI_NORMALS_OFFSET, sizeof(normals_hemi), normals_hemi );
    glBufferSubData( GL_ARRAY_BUFFER, HEMI_TEXCOORDS_OFFSET, sizeof(tex_coords_hemi), tex_coords_hemi );
    
    
    //---------------------------------------------------------------------
    
    // set up vertex arrays
    vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    
    vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(COLOR_OFFSET) );
    
    vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_TEXCOORDS_OFFSET) );
    
    // Set the value of the fragment shader texture sampler variable (myTextureSampler) to GL_TEXTURE0
    glUniform1i( glGetUniformLocation(program, "myTextureSampler"), 0 );
    
    //---------------------------------------------------------------------
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
}




//----------------------------------------------------------------------------

void
SetMaterial( vec4 ka, vec4 kd, vec4 ks, float s )
{
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
}

//----------------------------------------------------------------------------

void
SetLight( vec4 lpos, vec4 la, vec4 ld, vec4 ls )
{
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos);
    glUniform4fv( glGetUniformLocation(program, "AmbientLight"), 1, la);
    glUniform4fv( glGetUniformLocation(program, "DiffuseLight"), 1, ld);
    glUniform4fv( glGetUniformLocation(program, "SpecularLight"), 1, ls);
}

vec2
getClosest (vec2 objPos)
{
    vec2 closestXZ;
    float x = objPos.x; // I'm just passing the X and Z values and keeping them in a Vec 2
    float z = objPos.y; // so this says its the y component but just imagine it as if it were Z
    float decimal;
    if (x > 0.0)
    {
        decimal = (x - (float)((int)x));
        // this gets us the positive decimal of what we were looking for.
        if (decimal >= 0.5)
        {
            closestXZ.x = (((int)x)+1);
        }
        else
        {
            closestXZ.x = ((int)x);
        }
    }
    else if (x < 0.0)
    {
        decimal = (x - (float)((int)x));
        // this is the negative decimal value.
        if (decimal <= -0.5)
        {
            closestXZ.x = (((int)x)-1);
        }
        else
        {
            closestXZ.x = ((int)x);
        }
    }
    else
    {
        // we do nothing because x is 0 and we can just return that as an int.
        closestXZ.x = 0.0;
    }
    // now copy paste for z
    if (z > 0.0)
    {
        decimal = (z - (float)((int)z));
        if (decimal >= 0.5)
        {
            closestXZ.y = (((int)z)+1);
        }
        else
        {
            closestXZ.y = ((int)z);
        }
    }
    else if (z < 0.0)
    {
        decimal = (z - (float)((int)z));
        if (decimal <= -0.5)
        {
            closestXZ.y = (((int)z)-1);
        }
        else
        {
            closestXZ.y = ((int)z);
        }
    }
    else
    {
        closestXZ.y = 0.0;
    }
    return closestXZ;
}

int changeD(int curX, int curZ, int desX, int desZ, int direction, int otherbad)
{
    int retchoice;
    int choice1;//the left or right choice.
    int choice2;//the up or down choice
    
    // we cannot travel backwards, so we pass the current direction to ensure that the fmod(direction + 180) isnt our choice.
    int bad = (int)fmod((double)direction + 180.0, 360.0);
    printf("BAD IS %d!!! and OTHER BAD IS %d\n",bad,otherbad);
    // find the differences between current and destination, take the smaller of the differences, and make a relevant choice
    // to change your direction to get close to your destination.
    if ((curX - desX) < 0) // destination is to the right of current so we go there.
    {
        choice1 = cardinalright;
    }
    else if ((curX - desX ) > 0)
    {
        choice1 = cardinalleft;
    }
    else // you are right next to pac man, ignore this choice and go the other way
    {
        choice1 = bad;
    }
    if((curZ - desZ) < 0)// destination is DOWN from current, so we set direction to down
    {
        choice2 = cardinaldown;
    }
    else if((curZ - desZ) > 0)
    {
        choice2 = cardinalup;
    }
    else // the two are equal ignore this choice and take the other one.
    {
        choice2 = bad;
    }
    int diffx = std::abs(curX-desX);
    int diffz = std::abs(curZ-desZ);
    // here we find whichever difference is smaller, and use that to figure out the choice we make, if X is closer, then
    // we want to change to left or right direction to get there, BUT if that direction is bad, then we
    // simply set our choice to the other choice, because it's the only other choice that gets us closer.
    
    
    if (((choice1 == bad) || (choice1 == otherbad))&&((choice2 == bad)||(choice2 == otherbad)))
    {
        //none of the best options works! so we need to deprecate our direction, try up, then left, then down. until we get one
        if ((direction != bad) && (direction != otherbad))
        { retchoice = direction;}
        else if ((cardinalup != bad) && (cardinalup != otherbad)) {retchoice = cardinalup;}
        else if ((cardinalleft != bad) && (cardinalleft != otherbad)) {retchoice = cardinalleft;}
        //else if((cardinaldown != bad) && (cardinaldown != otherbad)) {retchoice = cardinaldown;}
        else {retchoice = cardinaldown;}
    }
    else if((choice1 == bad) || (choice1 == otherbad)) //one of them could be bad, so we check and change them accordingly, otherwise we just keep retchoice and use it.
    {
        retchoice = choice2;
    }
    else if((choice2 == bad) || (choice2 == otherbad))
    {
        retchoice = choice1;
    }
    else
    {
        if (diffz > diffx)// then we want to change the x direction left or right.
        {
            retchoice = choice1;
        }
        else
        {
            retchoice = choice2;
        }
    }
    /*
     if (diffz > diffx)// then we want to change the x direction left or right.
     {
     retchoice = choice1;
     }
     else
     {
     retchoice = choice2;
     }
     if ((retchoice == bad) && (bad == choice1))
     {
     retchoice = choice2;
     }
     else if ((retchoice == bad) && (bad == choice2))
     {
     retchoice = choice1;
     }
     */
    
    /*if ((retchoice == bad) && (bad == choice1))
     {
     retchoice = choice2;
     }
     else if ((retchoice == bad) && (bad == choice2))
     {
     retchoice = choice1;
     }*/
    /*if (diffx == diffz) // your choice doesnt reall matter here, so long as you dont go away from the destination...
     {
     // in this case the choice will be up or down, or right or left (not much help)
     if ((curX - desX) < 0)
     {
     //choice would be right because we want to go towards desX.
     }
     // we will default to going up if possible, within the idle function I will check the choice with
     // the available options, and deprecate choices from up > left > down if there are any issues with the choice.
     // hopefully that gives it an interesting interaction...
     }*/
    
    
    printf("THE DIRECTION CHOSEN WAS %d\n",retchoice);
    return retchoice;
}
bool isWhole(double numero)
{
    //printf("VALUE IS %f\n",goddamnbullshit);
    int castedshit = (int)numero;
    if ((int)numero == numero)
    {
        //printf("TRUE at value %d\n",numero);
        return true;
    }
    else
        return false;
}
int calcXPinky()
{
    int Xval;
    switch(PacD)
    {
        case cardinalright:
            Xval = (int)pacX + 16;
            break;
        case cardinalleft:
            Xval = (int)pacX - 16;
            break;
        case cardinalup:
            Xval = (int)pacZ;
            break;
        case cardinaldown:
            Xval = (int)pacZ;
        default:
            break;
    }
    return Xval;
}
int calcZPinky()
{
    int Zval;
    switch(PacD)
    {
        case cardinalright:
            Zval = (int)pacX;
            break;
        case cardinalleft:
            Zval = (int)pacX;
            break;
        case cardinalup:
            Zval = (int)pacZ - 16;
            break;
        case cardinaldown:
            Zval = (int)pacZ + 16;
        default:
            break;
    }
    return Zval;
}
//----------------------------------------------------------------------------


mat4 proj_matrix;

mat4 view_matrix;

float r = 7.5;

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //---- lights
    
    SetLight( vec4( 2.0, 2.0, 2.0, 1.0 ), vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    
    //---- camera intrinsic parameters
    
    //float left = -1.0;
    //float right = 1.0;
    //float bottom = -1.0;
    //float top = 1.0;
    //float zNear = 1.0;
    //float zFar = 15.0;
    
    float left = -2.0;
    float right = 2.0;
    float bottom = -2.0;
    float top = 2.0;
    float zNear = 1.0;
    float zFar = 60.0;
    
    //proj_matrix = Scale(2.0/(right-left), 2.0/(top-bottom), 2.0/(zNear-zFar));
    
    proj_matrix = Frustum( left, right, bottom, top, zNear, zFar );
    
    glUniformMatrix4fv( glGetUniformLocation( program, "projection" ), 1, GL_TRUE, proj_matrix );
    
    
    //---- camera extrinsic parameters
    
    float tr_y = Theta3[Yaxis]* M_PI/180.0;
    float tr_z = Theta3[Zaxis]* M_PI/180.0;
    //float eye_z = r * (cos(tr_z)) * cos(tr_y);
    //float eye_x = r * (cos(tr_z)) * sin(tr_y);
    //float eye_y = r * sin(tr_z);
    float eye_z = 15 +  (r * (cos(tr_z)) * cos(tr_y)); // change this for a more angled look I think 0 works best here
    float eye_x = (r * (cos(tr_z)) * sin(tr_y));
    float eye_y = 12.5 + (r * sin(tr_z));
    
    vec4 up = vec4(0.0, cos(tr_z), 0.0, 0.0);
    //cout << up << ' ' << normalize(up) << endl;
    
    view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 18.0, 1.0), vec4(0.0, cos(tr_z), 0.0, 0.0));
    
    glUniformMatrix4fv( glGetUniformLocation( program, "view" ), 1, GL_TRUE, view_matrix );
    
    
    //---- action
    
    
    SetLight( vec4( 0.0, 17.5, 0.0, 1.0 ), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.5, 0.5, 0.5, 1.0) );
    
    //---- bottom stage
    
    glBindTexture( GL_TEXTURE_2D, textures[1] ); // CILO for the stage
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    mat4 stage = Translate(0.0, -0.75, 2.0) * RotateY(270) * Scale((59 - 0 ) , 0.01, (53)); // at this spot the ghosts sit perfectly on top of the map
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, stage );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //---- hemisphere
    
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.9, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 pac =  Translate( pacX, pacY, pacZ ) * RotateY(PacD) * RotateX(-ThetaPac) * Scale(2.0, 2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pac );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //----- disc for inside pacman
    glBindTexture( GL_TEXTURE_2D, textures[5] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(0.5, 0.5, 0.5, 0.5), vec4(0.5, 0.2, 0.7, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 5.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    pac = Translate( pacX, pacY, pacZ ) * RotateY(PacD) * RotateX(-ThetaPac) * Scale(2.0, 0.001, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pac );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //---- hemisphere
    
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.9, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    pac =  Translate( pacX, pacY, pacZ ) * RotateY(PacD) * RotateX(ThetaPac) * Scale(2.0, -2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pac );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //----- disc for inside pacman
    
    glBindTexture( GL_TEXTURE_2D, textures[5] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(0.5, 0.5, 0.5, 0.5), vec4(0.5, 0.2, 0.7, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 5.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    pac = Translate( pacX, pacY, pacZ ) * RotateY(PacD) * RotateX(ThetaPac) * Scale(2.0, -0.001, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pac );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    
    //---- INKY top
    
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 inky =  Translate( InkyX, 0.75, InkyZ ) * RotateY(InkyD) * Scale(2.0, 2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, inky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //---- INKY face
    
    glBindTexture( GL_TEXTURE_2D, textures[3] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    inky = Translate( InkyX, 0.5, InkyZ ) * RotateY(InkyD) * Scale(2.0, 1.0, 2.0) * RotateY(someface + 90);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, inky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- INKY bottom
    
    glBindTexture( GL_TEXTURE_2D, textures[4] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    inky = Translate( InkyX, -0.5, InkyZ ) * RotateY(InkyD) * Scale(2.0, 1.0, 2.0) * RotateY(rotatebot);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, inky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- CLYDE top
    
    glBindTexture( GL_TEXTURE_2D, textures[6] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 clyde =  Translate( ClydeX, 0.75, ClydeZ ) *  RotateY(ClydeD) * Scale(2.0, 2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, clyde );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //---- CLYDE face
    
    glBindTexture( GL_TEXTURE_2D, textures[7] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    clyde = Translate( ClydeX, 0.5, ClydeZ ) * RotateY(ClydeD) * Scale(2.0, 1.0, 2.0) * RotateY(someface + 90);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, clyde );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- CLYDE bottom
    
    glBindTexture( GL_TEXTURE_2D, textures[8] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    clyde = Translate( ClydeX, -0.5, ClydeZ ) * RotateY(ClydeD) * Scale(2.0, 1.0, 2.0) * RotateY(rotatebot);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, clyde );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- BLINKY top
    glBindTexture( GL_TEXTURE_2D, textures[15] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 blinky =  Translate( BlinkyX, 0.75, BlinkyZ ) * RotateY(BlinkyD) * Scale(2.0, 2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, blinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //---- BLINKY face
    
    glBindTexture( GL_TEXTURE_2D, textures[16] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    //mat4 pinky =  Translate( PinkyX, 0.75, PinkyZ ) * RotateY(PinkyD) * Scale(2.0, 2.0, 2.0);
    //pinky = Translate( PinkyX, 0.5, PinkyZ ) * RotateY(PinkyD) * Scale(2.0, 1.0, 2.0) * RotateY(someface + 90);
    blinky =   Translate( BlinkyX, 0.5, BlinkyZ ) * RotateY(BlinkyD) * Scale(2.0, 1.0, 2.0) * RotateY(someface + 90);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, blinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- BLINKY bot
    
    glBindTexture( GL_TEXTURE_2D, textures[17] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    blinky = Translate( BlinkyX, -0.5, BlinkyZ ) * RotateY(BlinkyD) * Scale(2.0, 1.0, 2.0) * RotateY(rotatebot);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, blinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    
    //---- PINKY top
    glBindTexture( GL_TEXTURE_2D, textures[10] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 pinky =  Translate( PinkyX, 0.75, PinkyZ ) * RotateY(PinkyD) * Scale(2.0, 2.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(HEMI_TEXCOORDS_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesHemi );
    
    //---- PINKY face
    
    glBindTexture( GL_TEXTURE_2D, textures[11] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    pinky = Translate( PinkyX, 0.5, PinkyZ ) * RotateY(PinkyD) * Scale(2.0, 1.0, 2.0) * RotateY(someface + 90);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    //---- PINKY bot
    
    glBindTexture( GL_TEXTURE_2D, textures[12] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetMaterial( vec4(3.5, 3.5, 0.2, 1.0), vec4(0.2, 0.2, 0.1, 1.0), vec4(0.7, 0.2, 0.8, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    pinky = Translate( PinkyX, -0.5, PinkyZ ) * RotateY(PinkyD) * Scale(2.0, 1.0, 2.0) * RotateY(rotatebot);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, pinky );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_NORMALS_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_TEXCOORDS_OFFSET) );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    vec4 ka, kd, ks;
    vec4 lpos5 = vec4( 0.0, 0.0, 0.0, 1.0 );
    mat4 transform = 0;
    
    float s;
    
    //************************** Shape1 *************************************
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
    lpos5 = vec4( 0.0, 0.0, 0.0, 1.0 );
    ka = vec4(0.0, 0.0, 50.0, 1.0);
    kd = vec4(0.0, 0.0, 50.0, 1.0);
    ks = vec4(0.0, 0.0, 50.0, 1.0);
    s = 1.0;
    
    //glColor4f(10.0f,4.0f,4.0f,20.0f);
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, -23.4 ) * Scale(3.0, 1.0, 8.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape2 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(9.2, 0.0, -22.0 ) * Scale(9.0, 1.0, 5.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape3 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(20.3, 0.0, -22.0 ) * Scale(7.0, 1.0, 5.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape4 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-9.2, 0.0, -22.0 ) * Scale(9.0, 1.0, 5.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape5 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-20.3, 0.0, -22.0 ) * Scale(7.0, 1.0, 5.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape6 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(20.4, 0.0, -15.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape7 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-20.4, 0.0, -15.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //******************SHAPE8*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(12.0, 0.0, -9.0 ) * Scale(3.0, 1.0, 15.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    lpos5 = vec4( 0.0, 0.0, 0.0, 1.0 );
    ka = vec4(0.0, 0.0, 5.0, 1.0);
    kd = vec4(0.0, 0.0, 5.0, 1.0);
    ks = vec4(0.0, 0.0, 5.0, 1.0);
    s = 1.0;
    
    //glColor4f(10.0f,4.0f,4.0f,20.0f);
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(8.0, 0.0, -9.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //******************SHAPE9*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-12.0, 0.0, -9.0 ) * Scale(3.0, 1.0, 15.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-8.0, 0.0, -9.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //******************SHAPE10*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, -15.0 ) * Scale(15.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, -11.0 ) * Scale(3.0, 1.0, 7.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //****************SHAPE 11***********************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(12.0, 0.0, 6.0 ) * Scale(3.0, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //****************SHAPE 12***********************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-12.0, 0.0, 6.0 ) * Scale(3.0, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    
    //******************SHAPE13*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, 9.0 ) * Scale(15.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, 13.0 ) * Scale(3.0, 1.0, 7.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape14 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(9.2, 0.0, 15.0 ) * Scale(9.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape15 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-9.2, 0.0, 15.0 ) * Scale(9.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //******************SHAPE16*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, 21.0 ) * Scale(15.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, 25.0 ) * Scale(3.0, 1.0, 7.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //****************SHAPE 17***********************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(18.0, 0.0, 18.0 ) * Scale(3.0, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(20.0, 0.0, 15.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //****************SHAPE 18***********************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-18.0, 0.0, 18.0 ) * Scale(3.0, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-20.0, 0.0, 15.0 ) * Scale(7.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    
    //******************SHAPE19*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(14.0, 0.0, 27.0 ) * Scale(19.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(12.0, 0.0, 23.0 ) * Scale(3.0, 1.0, 7.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //******************SHAPE20*****************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-14.0, 0.0, 27.0 ) * Scale(19.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-12.0, 0.0, 23.0 ) * Scale(3.0, 1.0, 7.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape21 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(25.0, 0.0, 21.0 ) * Scale(4.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape22 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-25.0, 0.0, 21.0 ) * Scale(4.0, 1.0, 3.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    
    //************************** Shape23 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(21.8, 0.0, 6.0 ) * Scale(10.3, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape24 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-21.8, 0.0, 6.0 ) * Scale(10.3, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape25 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(21.8, 0.0, -6.0 ) * Scale(10.3, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //************************** Shape26 *************************************
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-21.8, 0.0, -6.0 ) * Scale(10.3, 1.0, 9.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    //************************** Shape27 *************************************
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    transform = Translate(0.0, 0.0, 3.5 ) * Scale(15, 1.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    transform = Translate(4.5, 0.0, -3.5 ) * Scale(6, 1.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    transform = Translate(-4.5, 0.0, -3.5 ) * Scale(6, 1.0, 2.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    transform = Translate(-6.5, 0.0, 0.0 ) * Scale(2.0, 1.0, 5.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    transform = Translate(6.5, 0.0, 0.0 ) * Scale(2.0, 1.0, 5.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //WALL #1
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(28.0, 0.0, 2.0 ) * Scale(2.0, 1.0, 59.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //Wall #2
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(-28.0, 0.0, 2.0 ) * Scale(2.0, 1.0, 59.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //Wall #3
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, -28.0 ) * Scale(57.93, 1.0, 2.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //Wall #4
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos5);
    
    transform = Translate(0.0, 0.0, 32.0 ) * Scale(57.93, 1.0, 2.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_NORMALS_OFFSET) );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
    
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    //**************BALLS*****************************
    
    ka = vec4(20.0, 0.0, 0.0, 1.0);
    kd = vec4(21.0, 0.0, 0.0, 1.0);
    ks = vec4(21.0, 0.0, 0.0, 1.0);
    s = 5.0;
    
    //Long Verticl Line
    
    int c = 0;
    z = -24.0;
    x = 15.0;
//        for(int k = 0; k < 17; k++)
//        {
//            glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
//            glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
//            glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
//            glUniform1f( glGetUniformLocation(program, "Shininess"), s );
//
//            transform = Translate(x, yArray[k], z ) * Scale(0.7 ,0.7, 0.7);
//
//            glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
//
//            glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
//            glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
//
//            glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
//            glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
//
//            glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
//            z += 3.0;
//        }
    //
    //
    //
    //    //Long Vertical Line #2
    //    c = -24.0;
    //    for(int k = 0; k < 17; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-15.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 3.0;
    //    }
    //
    //
    //
    //    //Long BOTTOM horizontal Line #1
    //    c = -25.0;
    //    for(int k = 0; k < 26; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 30 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //TOP RIGHT Horizontal
    //    c = 3.0;
    //    for(int k = 0; k < 12; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -26.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Top Left Horizontal
    //    c = -3.0;
    //    for(int k = 0; k < 12; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -26.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), ballIsThere );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //
    //    }
    //
    //    //Long Horizontal
    //    c = -25.0;
    //    int check = 0;
    //    for(int k = 0; k < 26; k++)
    //    {
    //        //Get rid of 21
    //        if(c == 15)
    //        {
    //            check -= 2;
    //        }
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, check, -18.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //        check = 0;
    //
    //    }
    //
    //    //Short right vertical
    //    c = -24.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(3.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Short left vertical
    //    c = -24.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-3.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    // vertical (9,-16)
    //    c = -16.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(9.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    // vertical (-9,-16)
    //    c = -16.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-9.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    // vertical (9,-12)
    //    c = 9.0;
    //    for(int k = 0; k < 4; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -12.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    //    // vertical (-9,-12)
    //    c = -9.0;
    //    for(int k = 0; k < 4; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -12.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (3,12)
    //    c = 3.0;
    //    for(int k = 0; k < 11; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 12.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (-3,12)
    //    c = -3.0;
    //    for(int k = 0; k < 11; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 12.0 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    //    //Horizontal (25,12)
    //    c = 12.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (-25,12)
    //    c = 12.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (3,14)
    //    c = 14.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(3, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal (-3,14)
    //    c = 14.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-3, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal (-13,18)
    //    c = -13.0;
    //    for(int k = 0; k < 6; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 18 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal (3,18)
    //    c = 3.0;
    //    for(int k = 0; k < 6; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 18 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (21,18)
    //    c = 21.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 18 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal (-21,18)
    //    c = -21.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 18 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    //    //Horizontal 21,20)
    //    c = 20.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(21.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal 21,20)
    //    c = 20.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-21.0, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal 17,24)
    //    c = 17.0;
    //    for(int k = 0; k < 5; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 24 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal -17,24)
    //    c = -17.0;
    //    for(int k = 0; k < 5; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 24 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    //
    //    //Horizontal 25,26)
    //    c = 26.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal -25,26)
    //    c = 26.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal 9,20)
    //    c = 20.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(9, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal -9,20)
    //    c = 20.0;
    //    for(int k = 0; k < 2; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-9, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal 3,24)
    //    c = 3.0;
    //    for(int k = 0; k < 5; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 24 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal -3,24)
    //    c = -3.0;
    //    for(int k = 0; k < 5; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, 24 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    //    //Horizontal 3,26)
    //    c = 26.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(3, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal -3,26)
    //    c = 26.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-3, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal 25,-24)
    //    c = -24.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal 25,-16)
    //    c = -16.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal 25,-24)
    //    c = -24.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal 25,-16)
    //    c = -16.0;
    //    for(int k = 0; k < 3; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(-25, 0.0, c ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //
    //    //Horizontal 17,-12)
    //    c = 17.0;
    //    for(int k = 0; k < 4; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -12 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c += 2.0;
    //    }
    //
    //    //Horizontal -17,-12)
    //    c = -17.0;
    //    for(int k = 0; k < 4; k++)
    //    {
    //        glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    //        glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    //        glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    //        glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    //
    //        transform = Translate(c, 0.0, -12 ) * Scale(0.7 ,0.7, 0.7);
    //
    //        glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    //
    //        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    //        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    //
    //        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    //        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    //
    //        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    //        c -= 2.0;
    //    }
    //
    
    
    
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    transform = Translate(-25, yArray[0], -25 ) * Scale(1.0 ,1.0, 1.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    transform = Translate(25, yArray[1], -25 ) * Scale(1.0 ,1.0, 1.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    transform = Translate(-25, yArray[2], 30 ) * Scale(1.0 ,1.0, 1.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
    transform = Translate(25, yArray[3], 30 ) * Scale(1.0 ,1.0, 1.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(SPHERE_NORMALS_OFFSET) );
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.9, 0.0, 1.0) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    glutSwapBuffers();
}


//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033:    // Escape key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
            
        case 'y':    //---- theta
            Axis = Yaxis;
            Theta3[Axis] += 5.0;
            Theta3[Axis] = fmod(Theta3[Axis], 360.0);
            glutPostRedisplay();
            break;
        case 'z':    //---- phi
            Axis = Zaxis;
            Theta3[Axis] += 5.0;
            Theta3[Axis] = fmod(Theta3[Axis], 360.0);
            glutPostRedisplay();
            break;
            
        case 'r':    //---- increase radius
            r += 0.5;
            glutPostRedisplay();
            break;
        case 'R':    //---- decrease radius
            r -= 0.5;
            glutPostRedisplay();
            break;
            
        case 'u':
            //view_matrix = default_view_matrix;
            Theta3[Xaxis] = 0.0;
            Theta3[Yaxis] = 0.0;
            Theta3[Zaxis] = 0.0;
            glutPostRedisplay();
            break;
        case 'A' :
            //frame = 0;
            //printf(" FRAME = %d",frame);
            //pacX -= 1.0;
            if (!gamestart)
            {
                gamestart = true;
            }
            PacD = 270.0;
            glutPostRedisplay();
            break;
        case 'D':
            //frame = 1;
            //printf(" FRAME = %d",frame);
            //pacZ -= 1.0;
            //pacX += 1.0;
            if (!gamestart)
            {
                gamestart = true;
            }
            PacD = cardinalright;// 90.0;
            glutPostRedisplay();
            break;
        case 'W' :
            //frame = 2;
            //printf(" FRAME = %d",frame);
            //pacZ -= 1.0;
            if (!gamestart)
            {
                gamestart = true;
            }
            PacD = cardinalup; //180.0;
            glutPostRedisplay();
            break;
        case 'S' :
            //frame = 3;
            //printf(" FRAME = %d",frame);
            //pacZ += 1.0;
            if (!gamestart)
            {
                gamestart = true;
            }
            PacD = cardinaldown; //0.0;
            glutPostRedisplay();
            break;
        case 'F' :
            gamestart = false;
            pacX = 0.0;
            pacY = 1.0;
            pacZ = 18.0;
            PacSpeed = 0.525;
            GhostSpeed = 0.5;
            glutPostRedisplay();
            break;
            
        default: printf ("   Special key %c == %d\n", key, key);
    }
}




//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
            case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
            case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
            case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------

void
idle( void )
{
    //Theta1[Axis] = fmod(Theta1[Axis]+1, 360.0);
    //Theta2[Axis] = fmod(Theta2[Axis]+2, 360.0);
    if (gamestart)
    {
        // first we need to get closest position on the grid.
        // then we check that position with direction and set the boolean "block"
        // if (!block) then we can move, and we handle with this switch otherwise
        // position is not updated.
        
        // the closest position function will help pacman/ghosts snap into the proper row/column for movement.
        vec2 currentPac = {pacX,pacZ};
        vec2 nearestInt = getClosest(currentPac);
        int x = nearestInt.x;
        int z = nearestInt.y;
        //pacX = (float)x;
        //pacZ = (float)z;
        //printf("x is %d, and z is %d\n",x,z);
        // Im going to program the long paths last so if you have multiple decisions to make it will happen first
        // so there are 60 unique spots that can branch out...
        // this is going to get ugly....
        // 23 horizontal paths to account for!
        // then 22 vertical paths??? I better do the math right @_@
        // actually I can just make the horizontals and verticals be inclusive and I can make this just horizontal/vertical...
        // if thats true then this will only take .... 21 horizontals, and 24 verticals, I'm also gonna add in
        // the intial if statement will handle if you go through the pipe area that warps you to the to the other side!
        if ((z == 0) && ((x > 26) || (x < -26)) && ((PacD == cardinalleft)||(PacD == cardinalright)))
        {
            if (pacX < 0)
            {
                pacX = -(pacX + 1);
            }
            else
            {
                pacX = -(pacX - 1);
            }
            block = false;
        }// gonna start horizontals from the top~~~
        {
            if (PacD == cardinalright) //quick shortcircuiting for the horizontal paths
            {
                // now to do the 21 horizontal checks here if none of them hit then we block
                // going to check every z coordinate first, because those are dead wringers for shortcircuits!
                // I can also set block to false at the beginning, so I dont need to set it in each case statement.
                //block = false;
                block = false;
                switch(z)
                {
                    case -26:// this is the main point, and the other points allow you to SNAP to position.
                    case -25:
                        if ((x < -25) || ((x >= -3) && (x < 3)) || (x >= 25)) // these are bad so we set the block
                        {
                            block = true; //otherwise we're good to move left or right!
                        }
                        else
                        {
                            pacZ = -26; //we snap him back to the most logical position and keep moving him on that path.
                        }
                        break;
                    case -19:// snap
                    case -18:// main
                    case -17:// snap
                        if ((x < -25) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -18;
                        }
                        break;
                    case -13:
                    case -12://main
                    case -11:
                        if ((x < -25) || ((x < -9)&&(x >= -15)) || ((x >= -3) && (x < 3)) || ((x >= 9) && (x < 15)) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -12;
                        }
                        break;
                    case -7:
                    case -6:
                    case -5:
                        if ((x < -9) || (x >= 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -6;
                        }
                        break;
                    case -1:
                    case 0://main
                    case 1:
                        if ((x >= -9) && (x < 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -0;
                        }
                        break;
                    case 5:
                    case 6://main
                    case 7:
                        if ((x < -9) || (x >= 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 6;
                        }
                        break;
                    case 11:
                    case 12://main
                    case 13:
                        if ((x < -25) || ((x >= -3) && (x < 3)) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 12;
                        }
                        break;
                    case 17:
                    case 18:
                    case 19:
                        if ((x < -25) || ((x < -15) && (x >= -21)) || ((x >= 15) && (x < 21)) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 18;
                        }
                        break;
                    case 23:
                    case 24://main
                    case 25:
                        if ((x < -25) || ((x < -9)&&(x >= -15)) || ((x >= -3)&&(x < 3)) || ((x >= 9) && (x < 15)) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 24;
                        }
                        break;
                    case 29:
                    case 30:
                        if ((x < -25) || (x >= 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 30;
                        }
                        break;
                    default:
                        block = true;
                        break;
                }
            }
            else if ( PacD == cardinalleft)
            {
                block = false;
                switch(z)
                {
                    case -26://main
                    case -25:
                        if ((x <= -25) || ((x > -3) && (x <= 3)) || (x > 25)) // these are bad so we set the block
                        {
                            block = true; //otherwise we're good to move left or right!
                        }
                        else
                        {
                            pacZ = -26;
                        }
                        break;
                    case -19:
                    case -18://main
                    case -17:
                        if ((x <= -25) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -18;
                        }
                        break;
                    case -13:
                    case -12://main
                    case -11:
                        if ((x <= -25) || ((x <= -9)&&(x > -15)) || ((x > -3) && (x <= 3)) || ((x > 9) && (x <= 15)) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -12;
                        }
                        break;
                    case -7:
                    case -6://main
                    case -5:
                        if ((x <= -9) || (x > 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = -6;
                        }
                        break;
                    case -1:
                    case 0://main
                    case 1:
                        if ((x > -9) && (x <= 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 0;
                        }
                        break;
                    case 5:
                    case 6://main
                    case 7:
                        if ((x <= -9) || (x > 9))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 6;
                        }
                        break;
                    case 11:
                    case 12://main
                    case 13:
                        if ((x <= -25) || ((x > -3) && (x <= 3)) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 12;
                        }
                        break;
                    case 17:
                    case 18:
                    case 19:
                        if ((x <= -25) || ((x <= -15) && (x > -21)) || ((x > 15) && (x <= 21)) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 18;
                        }
                        break;
                    case 23:
                    case 24://main
                    case 25:
                        if ((x <= -25) || ((x <= -9)&&(x > -15)) || ((x > -3)&&(x <= 3)) || ((x > 9) && (x <= 15)) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 24;
                        }
                        break;
                    case 29:
                    case 30:
                        if ((x <= -25) || (x > 25))
                        {
                            block = true;
                        }
                        else
                        {
                            pacZ = 30;
                        }
                        break;
                    default:
                        //pacX = x;
                        //pacZ = z;
                        block = true;
                        break;
                }
            }
            else if (PacD == cardinalup)
            {
                block = false;
                // now for the 24 vertical checks here if none are hit then we block the movement.
                int temp = x; //storing this so we can snap pacmans position here when needed.
                //x = (int)fabs((double)x);
                //printf("temp is %d and x is %d\n",temp,x);
                
                // since the maze is a mirror image accross the x = 0 plane, we can absolute value the x value and work with just
                // one half of the situations that can occur.
                switch(x)
                {
                    case 2:
                    case 3://main
                    case 4:
                        if ((z <= -26) || ((z > -18) && (z <= -12)) || ((z > -6) && (z <= 12 )) || ((z > 18) && (z <= 24)) || (z > 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 3;
                        }
                        break;
                    case 8:
                    case 9://main
                    case 10:
                        if ((z <= -18) || ((z > -12) && (z <= -6)) || ((z > 12) && (z <= 18)) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 9;
                        }
                        break;
                    case 14:
                    case 15://main
                    case 16:
                        if ((z <= -26) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 15;
                        }
                        break;
                    case 20:
                    case 21://main
                    case 22:
                        if ((z <= 18) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 21;
                        }
                        break;
                    case 24:
                    case 25://main
                        if ((z <= -26) || ((z > -12) && (z <= 12)) || ((z > 18) && (z <= 24)) || (z > 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 25;
                        }
                        break;
                    case -4:
                    case -3://main
                    case -2:
                        if ((z <= -26) || ((z > -18) && (z <= -12)) || ((z > -6) && (z <= 12 )) || ((z > 18) && (z <= 24)) || (z > 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -3;
                        }
                        break;
                    case -10:
                    case -9://main
                    case -8:
                        if ((z <= -18) || ((z > -12) && (z <= -6)) || ((z > 12) && (z <= 18)) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -9;
                        }
                        break;
                    case -16:
                    case -15://main
                    case -14:
                        if ((z <= -26) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -15;
                        }
                        break;
                    case -22:
                    case -21://main
                    case -20:
                        if ((z <= 18) || (z > 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -21;
                        }
                        break;
                    case -25://main
                    case -24:
                        if ((z <= -26) || ((z > -12) && (z <= 12)) || ((z > 18) && (z <= 24)) || (z > 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -25;
                        }
                        break;
                    default:
                        block = true;
                        break;
                }
            }
            else if (PacD == cardinaldown)
            {
                // now for the 24 vertical checks here if none are hit then we block the movement.
                int temp = x;
                //x = (int)fabs((double)x);
                // since the maze is a mirror image accross the x = 0 plane, we can absolute value the x value and work with just
                // one half of the situations that can occur.
                block = false;
                switch(x)
                {
                    case 2:
                    case 3://main
                    case 4:
                        if ((z < -26) || ((z >= -18) && (z < -12)) || ((z >= -6) && (z < 12 )) || ((z >= 18) && (z < 24)) || (z >= 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 3;
                        }
                        break;
                    case 8:
                    case 9://main
                    case 10:
                        if ((z < -18) || ((z >= -12) && (z < -6)) || ((z >= 12) && (z < 18)) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 9;
                        }
                        break;
                    case 14:
                    case 15://main
                    case 16:
                        if ((z < -26) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 15;
                        }
                        break;
                    case 20:
                    case 21://main
                    case 22:
                        if ((z < 18) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 21;
                        }
                        break;
                    case 24:
                    case 25://main
                        if ((z < -26) || ((z >= -12) && (z < 12)) || ((z >= 18) && (z < 24)) || (z >= 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = 25;
                        }
                        break;
                    case -2:
                    case -3://main
                    case -4://
                        if ((z < -26) || ((z >= -18) && (z < -12)) || ((z >= -6) && (z < 12 )) || ((z >= 18) && (z < 24)) || (z >= 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -3;
                        }
                        break;
                    case -8:
                    case -9://main
                    case -10:
                        if ((z < -18) || ((z >= -12) && (z < -6)) || ((z >= 12) && (z < 18)) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -9;
                        }
                        break;
                    case -14:
                    case -15://main
                    case -16:
                        if ((z < -26) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -15;
                        }
                        break;
                    case -22:
                    case -21://main
                    case -20:
                        if ((z < 18) || (z >= 24))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -21;
                        }
                        break;
                    case -25://main
                    case -24:
                        //                    if ((z <= -26) || ((z > -12) && (z <= 12)) || ((z > 18) && (z <= 24)) || (z > 30))
                        if ((z < -26) || ((z >= -12) && (z < 12)) || ((z >= 18) && (z < 24)) || (z >= 30))
                        {
                            block = true;
                        }
                        else
                        {
                            pacX = -25;
                        }
                        break;
                    default:
                        block = true;
                        break;
                }
            }
            else
            {
                // this shouldnt be hit ever?
                //block = false;
                //pacX = x;
                //pacZ = z;
            }
            if (!block) //if you are not blocked then you can move in whatever direction.
            {
                switch (PacD) {
                    case (cardinalup):
                        pacZ -= PacSpeed;
                        break;
                    case (cardinalright):
                        pacX += PacSpeed;
                        break;
                    case (cardinaldown):
                        pacZ += PacSpeed;
                        break;
                    case (cardinalleft):
                        pacX -= PacSpeed;
                        break;
                    default:
                        //lol get owned
                        break;
                }
            }
        }
        /*
         if (PacD == cardinaldown)
         {
         pacZ += PacSpeed;
         }
         if (PacD == cardinalup)
         {
         pacZ -= PacSpeed;
         }
         if (PacD == cardinalright)
         {
         pacX += PacSpeed;
         }
         if (PacD == cardinalleft)
         {
         pacX -= PacSpeed;
         }*/
    }
    Theta4[Xaxis] = fmod(Theta4[Xaxis]+0.5, 360.0);
    
    ThetaPac = fmod(ThetaPac + 2 , 45); //controls how pacmans mouth works
    
    rotatebot = fmod(rotatebot + 5.0, 180);
    
    if (((pacX > 25) || (pacX < -25)) && !movePinky)
    {
        spawnPinky = true;
    }
    if ((pacZ < 0) && (!moveInky))
    {
        spawnInky = true;
    }
    if (moveBlinky && moveInky && movePinky && (!moveClyde))
    {
        for (int i = 0; i < 10000; i++)
        {
            //does nothing but waste time before spawning Clyde?
        }
        spawnClyde = true;
    }
    if (spawnBlinky)
    {
        printf("spawningblinky\n");
        spawnBlinky = false; // we set it to false unless he's dead (for this project I won't bother with respawning ghosts...)
        moveBlinky = true;
        BlinkyX = SpawnX;
        BlinkyZ = SpawnZ; //set him into his position...
        BlinkyD = cardinalleft; //from what I'm reading online they all start going left.
        // set his position (Blinky starts in the right spot)
        // check his position and his target. use that to set his initial direction BlinkyD which should always be right.
    }
    if (spawnPinky)
    {
        printf("spawningpinky\n");
        movePinky = true;
        spawnPinky = false;
        PinkyX = SpawnX;
        PinkyZ = SpawnZ;
        PinkyD = cardinalleft;
    }
    if (spawnInky)
    {
        printf("spawningInky\n");
        moveInky = true;
        spawnInky = false;
        InkyX = SpawnX;
        InkyZ = SpawnZ;
        InkyD = cardinalleft;
    }
    if (spawnClyde)
    {
        printf("spawningClyde\n");
        moveClyde = true;
        spawnClyde = false;
        ClydeX = SpawnX;
        ClydeZ = SpawnZ;
        ClydeD = cardinalleft;
    }
    if (gamestart) //this is just a seperate block to handle ghost movement.
    {
        //vec2 BPos = {BlinkyX,BlinkyZ}; // current position, will be passed to find his direction
        //vec2 Bdest; //this is his destination, which changes based on the move types; default, scatter, or chase.
        
        //check if BlinkyX is a whole number... then send to a switch statement on the X value, and compare with Z values to do different shit.
        if ((moveBlinky) && (isWhole(BlinkyX)))
        {
            switch((int)BlinkyX)
            {
                case 3:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -6:
                            case 18:
                                if (BlinkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                    else if (chasemove)
                                    {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(BlinkyD == cardinalup){BlinkyD = cardinalright;}
                                else if (BlinkyD == cardinalleft){BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -3:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -6:
                            case 18:
                                if (BlinkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                    else if (chasemove)
                                    {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(BlinkyD == cardinalup){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright)
                                {BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 9:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalleft);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalleft);}
                                break;
                            case 6:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalright);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalright);}
                                break;
                            case 12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalup);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(BlinkyD == cardinaldown){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright){BlinkyD = cardinalup;}
                                break;
                            case -6:
                                if(BlinkyD == cardinalup){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright){BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -9:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalright);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalright);}
                                break;
                            case 6:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalleft);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalleft);}
                                break;
                            case 12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalup);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(BlinkyD == cardinaldown){BlinkyD = cardinalright;}
                                else if(BlinkyD == cardinalleft) {BlinkyD = cardinalup;}
                                break;
                            case -6:
                                if(BlinkyD == cardinalup){BlinkyD = cardinalright;}
                                else if(BlinkyD == cardinalleft) {BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 15:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalup);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,5);}//these can go in any direction...
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalleft);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalleft);}
                                break;
                            case 18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalright);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalright);}
                                break;
                            case 24:
                                if (BlinkyD == cardinaldown){BlinkyD = cardinalright;}
                                else if(BlinkyD == cardinalleft) {BlinkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -15:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalup);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,5);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalright);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalright);}
                                break;
                            case 18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalleft);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalleft);}
                                break;
                            case 24:
                                if (BlinkyD == cardinaldown){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright) {BlinkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 21:
                    if (BlinkyZ == 24)
                    {
                        if (defaultmove){BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                        else if (chasemove) {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                    }
                    else if (BlinkyZ == 18)
                    {
                        if (BlinkyD == cardinalup){BlinkyD = cardinalright;}
                        else if(BlinkyD == cardinalleft) {BlinkyD = cardinaldown;}
                    }
                    break;
                case -21:
                    if (BlinkyZ == 24)
                    {
                        if (defaultmove){BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinaldown);}
                        else if (chasemove) {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinaldown);}
                    }
                    else if (BlinkyZ == 18)
                    {
                        if (BlinkyD == cardinalup){BlinkyD = cardinalleft;}
                        else if(BlinkyD == cardinalright){BlinkyD = cardinaldown;}
                    }
                    break;
                case 25:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalright);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalright);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (BlinkyD == cardinaldown){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright) {BlinkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (BlinkyD == cardinalup){BlinkyD = cardinalleft;}
                                else if(BlinkyD == cardinalright) {BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -25:
                    if (isWhole(BlinkyZ))
                    {
                        switch((int)BlinkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,20,-22,BlinkyD,cardinalleft);}
                                else if (chasemove)
                                {BlinkyD = changeD((int)BlinkyX,(int)BlinkyZ,(int)pacX,(int)pacZ,BlinkyD,cardinalleft);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (BlinkyD == cardinaldown){BlinkyD = cardinalright;}
                                else if(BlinkyD == cardinalleft) {BlinkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (BlinkyD == cardinalup){BlinkyD = cardinalright;}
                                else if(BlinkyD == cardinalleft) {BlinkyD = cardinaldown;}
                            default:
                                break;
                        }
                    }
                default:
                    break;
            }
        }
        //FOR PINKY
        if ((movePinky) && (isWhole(PinkyX)))
        {
            switch((int)PinkyX)
            {
                case 3:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -6:
                            case 18:
                                if (PinkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                                    else if (chasemove)
                                    {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(PinkyD == cardinalup){PinkyD = cardinalright;}
                                else if (PinkyD == cardinalleft){PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -3:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -6:
                            case 18:
                                if (PinkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                                    else if (chasemove)
                                    {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(PinkyD == cardinalup){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright)
                                {PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 9:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalleft);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalleft);}
                                break;
                            case 6:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalright);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalright);}
                                break;
                            case 12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)BlinkyZ,-20,-22,BlinkyD,cardinaldown);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalup);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(PinkyD == cardinaldown){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright){PinkyD = cardinalup;}
                                break;
                            case -6:
                                if(PinkyD == cardinalup){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright){PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -9:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalright);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalright);}
                                break;
                            case 6:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalleft);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalleft);}
                                break;
                            case 12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalup);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(PinkyD == cardinaldown){PinkyD = cardinalright;}
                                else if(PinkyD == cardinalleft) {PinkyD = cardinalup;}
                                break;
                            case -6:
                                if(PinkyD == cardinalup){PinkyD = cardinalright;}
                                else if(PinkyD == cardinalleft) {PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 15:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalup);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,5);}//these can go in any direction...
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalleft);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalleft);}
                                break;
                            case 18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalright);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalright);}
                                break;
                            case 24:
                                if (PinkyD == cardinaldown){PinkyD = cardinalright;}
                                else if(PinkyD == cardinalleft) {PinkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -15:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalup);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,5);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalright);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalright);}
                                break;
                            case 18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalleft);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalleft);}
                                break;
                            case 24:
                                if (PinkyD == cardinaldown){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright) {PinkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 21:
                    if (PinkyZ == 24)
                    {
                        if (defaultmove){PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                        else if (chasemove){PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                    }
                    else if (PinkyZ == 18)
                    {
                        if (PinkyD == cardinalup){PinkyD = cardinalright;}
                        else if(PinkyD == cardinalleft) {PinkyD = cardinalright;}
                    }
                    break;
                case -21:
                    if (PinkyZ == 24)
                    {
                        if (defaultmove){PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinaldown);}
                        else if (chasemove){PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinaldown);}
                    }
                    else if (PinkyZ == 18)
                    {
                        if (PinkyD == cardinalup){PinkyD = cardinalleft;}
                        else if(PinkyD == cardinalright){PinkyD = cardinalleft;}
                    }
                    break;
                case 25:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalright);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalright);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (PinkyD == cardinaldown){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright) {PinkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (PinkyD == cardinalup){PinkyD = cardinalleft;}
                                else if(PinkyD == cardinalright) {PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -25:
                    if (isWhole(PinkyZ))
                    {
                        switch((int)PinkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,-20,-22,PinkyD,cardinalleft);}
                                else if (chasemove)
                                {PinkyD = changeD((int)PinkyX,(int)PinkyZ,calcXPinky(),calcZPinky(),PinkyD,cardinalleft);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (PinkyD == cardinaldown){PinkyD = cardinalright;}
                                else if(PinkyD == cardinalleft) {PinkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (PinkyD == cardinalup){PinkyD = cardinalright;}
                                else if(PinkyD == cardinalleft) {PinkyD = cardinaldown;}
                            default:
                                break;
                        }
                    }
                default:
                    break;
            }
        }
        // FOR INKY
        if ((moveInky) && (isWhole(InkyX)))
        {
            switch((int)InkyX)
            {
                case 3:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -6:
                            case 18:
                                if (InkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                    else if (chasemove)
                                    {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(InkyD == cardinalup){InkyD = cardinalright;}
                                else if (InkyD == cardinalleft){InkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -3:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -6:
                            case 18:
                                if (InkyD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                    else if (chasemove)
                                    {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(InkyD == cardinalup){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright)
                                {InkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 9:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalleft);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalleft);}
                                break;
                            case 6:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalright);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalright);}
                                break;
                            case 12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalup);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(InkyD == cardinaldown){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright){InkyD = cardinalup;}
                                break;
                            case -6:
                                if(InkyD == cardinalup){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright){InkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -9:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case 0:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalright);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalright);}
                                break;
                            case 6:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalleft);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalleft);}
                                break;
                            case 12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalup);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalup);}
                                break;
                            case -12:
                            case 24:
                                if(InkyD == cardinaldown){InkyD = cardinalright;}
                                else if(InkyD == cardinalleft) {InkyD = cardinalup;}
                                break;
                            case -6:
                                if(InkyD == cardinalup){InkyD = cardinalright;}
                                else if(InkyD == cardinalleft) {InkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 15:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalup);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,5);}//these can go in any direction...
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalleft);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalleft);}
                                break;
                            case 18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalright);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalright);}
                                break;
                            case 24:
                                if (InkyD == cardinaldown){InkyD = cardinalright;}
                                else if(InkyD == cardinalleft) {InkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -15:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -26:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalup);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalup);}
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,5);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,5);}
                                break;
                            case -12:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalright);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalright);}
                                break;
                            case 18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalleft);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalleft);}
                                break;
                            case 24:
                                if (InkyD == cardinaldown){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright) {InkyD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 21:
                    if (InkyZ == 24)
                    {
                        if (defaultmove){InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                        else if (chasemove){InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                    }
                    else if (InkyZ == 18)
                    {
                        if (InkyD == cardinalup){InkyD = cardinalright;}
                        else if(InkyD == cardinalleft) {InkyD = cardinalright;}
                    }
                    break;
                case -21:
                    if (InkyZ == 24)
                    {
                        if (defaultmove){InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinaldown);}
                        else if (chasemove){InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinaldown);}
                    }
                    else if (InkyZ == 18)
                    {
                        if (InkyD == cardinalup){InkyD = cardinalleft;}
                        else if(InkyD == cardinalright){InkyD = cardinalleft;}
                    }
                    break;
                case 25:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalright);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalright);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (InkyD == cardinaldown){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright) {InkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (InkyD == cardinalup){InkyD = cardinalleft;}
                                else if(InkyD == cardinalright) {InkyD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -25:
                    if (isWhole(InkyZ))
                    {
                        switch((int)InkyZ)
                        {
                            case -18:
                                if (defaultmove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,14,27,InkyD,cardinalleft);}
                                else if (chasemove)
                                {InkyD = changeD((int)InkyX,(int)InkyZ,(pacX-BlinkyX),(pacZ-BlinkyZ),InkyD,cardinalleft);}
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (InkyD == cardinaldown){InkyD = cardinalright;}
                                else if(InkyD == cardinalleft) {InkyD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (InkyD == cardinalup){InkyD = cardinalright;}
                                else if(InkyD == cardinalleft) {InkyD = cardinaldown;}
                            default:
                                break;
                        }
                    }
                default:
                    break;
            }
        }
        // FOR CLYDE
        if ((moveClyde) && (isWhole(ClydeX)))
        {
            switch((int)ClydeX)
            {
                case 3:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -6:
                            case 18:
                                if (ClydeD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                    else if (chasemove)
                                    {
                                        if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                        {
                                            ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                        }
                                        else{
                                            ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                        }
                                    }
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                    }
                                }
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(ClydeD == cardinalup){ClydeD = cardinalright;}
                                else if (ClydeD == cardinalleft){ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -3:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -6:
                            case 18:
                                if (ClydeD == cardinaldown)
                                {
                                    if (defaultmove)
                                    {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                    else if (chasemove)
                                    {
                                        if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                        {
                                            ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                        }
                                        else{
                                            ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                        }
                                    }
                                }
                                break; //unless its in random mode it should only go left and right
                            case -18:
                            case 30:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                    }
                                }
                                break;
                            case -12:
                            case -26:
                            case 12:
                            case 24:
                                if(ClydeD == cardinalup){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright)
                                {ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 9:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case 0:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalleft);
                                    }
                                }
                                break;
                            case 6:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalright);
                                    }
                                }
                                break;
                            case 12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                    }
                                }
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalup);
                                    }
                                }
                                break;
                            case -12:
                            case 24:
                                if(ClydeD == cardinaldown){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright){ClydeD = cardinalup;}
                                break;
                            case -6:
                                if(ClydeD == cardinalup){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright){ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -9:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case 0:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalright);
                                    }
                                }
                                break;
                            case 6:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalleft);
                                    }
                                }
                                break;
                            case 12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                                    }
                                }
                                break;
                            case 18:
                            case -18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalup);
                                    }
                                }
                                break;
                            case -12:
                            case 24:
                                if(ClydeD == cardinaldown){ClydeD = cardinalright;}
                                else if(ClydeD == cardinalleft) {ClydeD = cardinalup;}
                                break;
                            case -6:
                                if(ClydeD == cardinalup){ClydeD = cardinalright;}
                                else if(ClydeD == cardinalleft) {ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 15:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -26:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalup);
                                    }
                                }
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,5);}//these can go in any direction...
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,5);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,5);
                                    }
                                }
                                break;
                            case -12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalleft);
                                    }
                                }
                                break;
                            case 18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalright);
                                    }
                                }
                                break;
                            case 24:
                                if (ClydeD == cardinaldown){ClydeD = cardinalright;}
                                else if(ClydeD == cardinalleft) {ClydeD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -15:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -26:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalup);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalup);
                                    }
                                }
                                break;
                            case -18:
                            case 0:
                            case 12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,5);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,5);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,5);
                                    }
                                }
                                break;
                            case -12:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalright);
                                    }
                                }
                                break;
                            case 18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalleft);
                                    }
                                }
                                break;
                            case 24:
                                if (ClydeD == cardinaldown){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright) {ClydeD = cardinalup;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case 21:
                    if (ClydeZ == 24)
                    {
                        if (defaultmove){ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                        else if (chasemove){
                            if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                            {
                                ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                            }
                            else{
                                ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                            }
                        }
                    }
                    else if (ClydeZ == 18)
                    {
                        if (ClydeD == cardinalup){ClydeD = cardinalright;}
                        else if(ClydeD == cardinalleft) {ClydeD = cardinalright;}
                    }
                    break;
                case -21:
                    if (ClydeZ == 24)
                    {
                        if (defaultmove){ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);}
                        else if (chasemove){
                            if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                            {
                                ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinaldown);
                            }
                            else{
                                ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinaldown);
                            }
                        }
                    }
                    else if (ClydeZ == 18)
                    {
                        if (ClydeD == cardinalup){ClydeD = cardinalleft;}
                        else if(ClydeD == cardinalright){ClydeD = cardinalleft;}
                    }
                    break;
                case 25:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalright);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalright);
                                    }
                                }
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (ClydeD == cardinaldown){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright) {ClydeD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (ClydeD == cardinalup){ClydeD = cardinalleft;}
                                else if(ClydeD == cardinalright) {ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                        break;
                    }
                    break;
                case -25:
                    if (isWhole(ClydeZ))
                    {
                        switch((int)ClydeZ)
                        {
                            case -18:
                                if (defaultmove)
                                {ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);}
                                else if (chasemove)
                                {
                                    if ((abs(ClydeX-pacX) < 4) || (abs(ClydeZ - pacZ) < 4))
                                    {
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,-14,27,ClydeD,cardinalleft);
                                    }
                                    else{
                                        ClydeD = changeD((int)ClydeX,(int)ClydeZ,(pacX),(pacZ),ClydeD,cardinalleft);
                                    }
                                }
                                break;
                            case 30:
                            case -12:
                            case 18:
                                if (ClydeD == cardinaldown){ClydeD = cardinalright;}
                                else if(ClydeD == cardinalleft) {ClydeD = cardinalup;}
                                break;
                            case 24:
                            case 12:
                            case -26:
                                if (ClydeD == cardinalup){ClydeD = cardinalright;}
                                else if(ClydeD == cardinalleft) {ClydeD = cardinaldown;}
                            default:
                                break;
                        }
                    }
                default:
                    break;
            }
        }
        // THIS HANDLES THEM WRAPPING AROUND THE MAP
        if ((BlinkyZ == 0) && ((BlinkyX > 26) || (BlinkyX < -26)) && ((BlinkyD == cardinalleft)||(BlinkyD == cardinalright)))
        {
            if (BlinkyX < 0)
            {
                BlinkyX = -(BlinkyX + 1);
            }
            else
            {
                BlinkyX = -(BlinkyX - 1);
            }
        }
        if ((PinkyZ == 0) && ((PinkyX > 26) || (PinkyX < -26)) && ((PinkyD == cardinalleft)||(PinkyD == cardinalright)))
        {
            if (PinkyX < 0)
            {
                PinkyX = -(PinkyX + 1);
            }
            else
            {
                PinkyX = -(PinkyX - 1);
            }
        }
        if ((InkyZ == 0) && ((InkyX > 26) || (InkyX < -26)) && ((InkyD == cardinalleft)||(InkyD == cardinalright)))
        {
            if (InkyX < 0)
            {
                InkyX = -(InkyX + 1);
            }
            else
            {
                InkyX = -(InkyX - 1);
            }
        }
        if ((ClydeZ == 0) && ((ClydeX > 26) || (ClydeX < -26)) && ((ClydeD == cardinalleft)||(ClydeD == cardinalright)))
        {
            if (ClydeX < 0)
            {
                ClydeX = -(ClydeX + 1);
            }
            else
            {
                ClydeX = -(ClydeX - 1);
            }
        }
        
        /*if (defaultmove)
         {
         BlinkyD = cardinalright;
         //Bdest = {20.0,-22.0}; // where he wants to go when the game starts.
         
         // set direction that will get him to the that point
         // when they spawn they always have a set direction they will start heading in so I'm going to hardcode that
         // to start the movement Blinky will go right, Inky will go right, Pinky will go left, and Clyde will also go left.
         }
         else if(chasemove)
         {
         //Bdest = {pacX, pacZ};
         }
         else if(scatter)
         {
         
         }*/
        if (moveBlinky)
        {
            switch ((int)BlinkyD)
            {
                case (cardinalup):
                    BlinkyZ -= GhostSpeed;
                    break;
                case (cardinalright):
                    BlinkyX += GhostSpeed;
                    break;
                case (cardinaldown):
                    BlinkyZ += GhostSpeed;
                    break;
                case (cardinalleft):
                    BlinkyX -= GhostSpeed;
                    break;
                default:
                    //lol get fucked
                    break;
            }
        }
        if (movePinky)
        {
            switch ((int)PinkyD)
            {
                case (cardinalup):
                    PinkyZ -= GhostSpeed;
                    break;
                case (cardinalright):
                    PinkyX += GhostSpeed;
                    break;
                case (cardinaldown):
                    PinkyZ += GhostSpeed;
                    break;
                case (cardinalleft):
                    PinkyX -= GhostSpeed;
                    break;
                default:
                    //lol get fucked
                    break;
            }
        }
        if (moveInky)
        {
            switch ((int)InkyD)
            {
                case (cardinalup):
                    InkyZ -= GhostSpeed;
                    break;
                case (cardinalright):
                    InkyX += GhostSpeed;
                    break;
                case (cardinaldown):
                    InkyZ += GhostSpeed;
                    break;
                case (cardinalleft):
                    InkyX -= GhostSpeed;
                    break;
                default:
                    //lol get fucked
                    break;
            }
        }
        if (moveClyde)
        {
            switch ((int)ClydeD)
            {
                case (cardinalup):
                    ClydeZ -= GhostSpeed;
                    break;
                case (cardinalright):
                    ClydeX += GhostSpeed;
                    break;
                case (cardinaldown):
                    ClydeZ += GhostSpeed;
                    break;
                case (cardinalleft):
                    ClydeX -= GhostSpeed;
                    break;
                default:
                    //lol get fucked
                    break;
            }
        }
        
        
        
        printf("Pac X position is: %f\n", pacX);
        printf("Pac Y position is: %f\n", pacZ);
        //int nexty = 0;
        if(round(pacX) == -25 && round(pacZ) == -25 )
        {
            if(yArray[0] == 0)
            {
                yArray[0] -= 2;
                printf("The ball was eaten !!!!!!!!!!!!\n");
                ballsEaten++;
                printf("Number of balls eaten: %d", ballsEaten);
            }
        }
        
        
        if(round(pacX) == 25 && round(pacZ) == -25)
        {
            
            if(yArray[1] == 0)
            {
                yArray[1] -= 2;
                printf("The ball was eaten !!!!!!!!!!!!\n");
                ballsEaten++;
                printf("Number of balls eaten: %d", ballsEaten);
            }
        }
        
        if(round(pacX) == -25 && round(pacZ) == 30)
        {
            
            if(yArray[2] == 0)
            {
                yArray[2] -= 2;
                printf("The ball was eaten !!!!!!!!!!!!\n");
                ballsEaten++;
                printf("Number of balls eaten: %d", ballsEaten);
            }
            
        }
        
        if(round(pacX) == 25 && round(pacZ) == 30)
        {
            
            if(yArray[3] == 0)
            {
                yArray[3] -= 2;
                printf("The ball was eaten !!!!!!!!!!!!\n");
                ballsEaten++;
                printf("Number of balls eaten: %d", ballsEaten);
            }
        }
        if (((abs(pacX-BlinkyX) < 0.5) && (abs(pacZ-BlinkyZ) < 0.5)) || ((abs(pacX-PinkyX) < 0.5) && (abs(pacZ-PinkyZ) < 0.5)) || ((abs(pacX-InkyX) < 0.5) && (abs(pacZ-InkyZ) < 0.5)) || ((abs(pacX-ClydeX) < 0.5) && (abs(pacZ-ClydeZ) < 0.5)))
        {
            PacSpeed = 0;
            GhostSpeed = 0;
            pacY -= .075;
            //pacY += .5; //when you win.
        }
        
        if (ballsEaten == 4)
        {
            PacSpeed = 0;
            GhostSpeed = 0;
            pacY += .5;
        }
        
        if (((abs(pacX-BlinkyX) < 0.5) && (abs(pacZ-BlinkyZ) < 0.5)) || ((abs(pacX-PinkyX) < 0.5) && (abs(pacZ-PinkyZ) < 0.5)) || ((abs(pacX-InkyX) < 0.5) && (abs(pacZ-InkyZ) < 0.5)) || ((abs(pacX-ClydeX) < 0.5) && (abs(pacZ-ClydeZ) < 0.5)))
        {
            PacSpeed = 0;
            GhostSpeed = 0;
            pacY -= .075;
            //pacY += .5; //when you win.
        }
        
        
        
        
        
        
    }
    
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
reshape( int w, int h )
{
    glViewport(0,0,w,h);
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 1000, 800 );
    glutCreateWindow( "Color Cube" );
    
    init();
    
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    glutIdleFunc( idle );
    
    glutMainLoop();
    return 0;
}

#endif
