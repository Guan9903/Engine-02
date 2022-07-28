/******************************************************************************
Author:Rich Davison
Description: Some random variables and functions, for lack of a better place 
to put them.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // ! WIN32_LEAN_AND_MEAN

#ifndef  NOMINMAX
#define NOMINMAX
#endif // ! NOMINMAX


//It's pi(ish)...
static const float		PI = 3.14159265358979323846f;	

//It's pi...divided by 360.0f!
static const float		PI_OVER_360 = PI / 360.0f;

//Radians to degrees
static inline float RadToDeg(const float deg)	{
	return deg * 180.0f / PI;
};

//Degrees to radians
static inline float DegToRad(const float rad)	{
	return rad * PI / 180.0f;
};

#define SHADERDIR	"../Shaders/"
#define MESHDIR		"../Meshes/"
#define TEXTUREDIR  "../Textures/"
#define SOUNDSDIR	"../Sounds/"

#define SHADERDIR_MYSHADER		"../Blank Project/MyShader/"
#define TEXTUREDIR_SKYBOXES		"../Textures/SkyBoxes/"
#define TEXTUREDIR_HEIGHTMAPS	"../Textures/HeightMaps/"
#define TEXTUREDIR_NORMALMAPS	"../Textures/NormalMaps/"
#define TEXTUREDIR_MODELS		"../Textures/Models/"