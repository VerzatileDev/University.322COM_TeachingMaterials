# Session 2 - Ray Casting

#### Table of Contents
1. [Sphere Ray Casting Solution](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Sphere-Ray-Casting-Solution)
2. [Redesign data class](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Redesign-data-class)
3. [Add triangle class](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Add-triangle-class)
4. [Rendering complex shape](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Rendering-complex-shape)
5. [Example of mesh class](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Example-of-mesh-class)
6. [Add screen display using SDL2 Library](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202#Add-screen-display-using-SDL2-Library)

## Sphere Ray Casting Solution

If you have trouble to implement sphere ray intersection. Here is the function codes.

```C++
//center is the sphere center position
//orig is the origin of the ray. You set it up as (0,0,0)
//dir is the direction of the ray
//t is the return parameter value of intersection. Range from [0,1]
bool intersectSphere(vec3 center, vec3 orig, vec3 dir, float radius, float &t)
{
	float t0, t1; // solutions for t if the ray intersects 

	// geometric solution  // vector dir has to be normalize, length is 1.0
	vec3 L = center - orig;
	float tca = dot(L, dir);
	if (tca < 0) return false;
	float d = dot(L,L) - tca * tca;
	if (d > (radius*radius)) return false;

	float thc = sqrt(radius*radius - d);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) return false; // both t0 and t1 are negative 
	}

	t = t0;
	return true;
}
```

A simple sphere class (header file):
```C++
#pragma once
#include <glm/glm.hpp>

using namespace glm;

class Sphere
{
private:
	float radius;
	vec3  center;
	vec3 mycolor;
public:
	Sphere(float,vec3,vec3);
	~Sphere();
	float getRadius(void);
	vec3 getCenter(void);
	vec3 getMyColor(void);
};
```

CPP file for the sphere class
```C++
#include "stdafx.h"
#include "sphere.h"

Sphere::Sphere(float r, vec3 cen, vec3 col) {
	radius = r;
	center = cen;
	mycolor = col;
}

Sphere::~Sphere()
{
}

float Sphere::getRadius(void)
{
	return radius;
}

vec3 Sphere::getCenter(void)
{
	return center;
}

vec3 Sphere::getMyColor(void)
{
	return mycolor;
}
```

Intersection points order sorting codes. So, the closest sphere will display the full sphere. 

```C++
	vector<float> t_arr;
	vector<vec3> color_arr;
	
    for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			t_arr.clear();
			color_arr.clear();

			PixelNdx = (x + 0.5) / (float)WIDTH;
			PixelNdy = (y + 0.5) / (float)HEIGHT;
			PixelRdx = 2 * PixelNdx - 1.0;
			PixelRdy = 1.0 - 2 * PixelNdy;
			PixelRdx = PixelRdx * Iar;

			PCameraX = PixelRdx*tanvalue;
			PCameraY = PixelRdy*tanvalue;

			ttvec.x = PCameraX;
			ttvec.y = PCameraY;
			ttvec.z = -1.0;
			//dir need to be normalized
			dir = normalize(ttvec);

			org.x = 0.0; org.y = 0.0; org.z = 0.0;
			Intersection = intersectSphere(redsphere.getCenter(), org, dir, redsphere.getRadius(), t);
			if (Intersection)
			{
				t_arr.push_back(t);
				color_arr.push_back(redsphere.getMyColor());
			}

			Intersection = intersectSphere(yellowsphere.getCenter(), org, dir, yellowsphere.getRadius(), t);
			if (Intersection)
			{
				t_arr.push_back(t);
				color_arr.push_back(yellowsphere.getMyColor());
			}

			Intersection = intersectSphere(bluesphere.getCenter(), org, dir, bluesphere.getRadius(), t);
			if (Intersection)
			{
				t_arr.push_back(t);
				color_arr.push_back(bluesphere.getMyColor());
			}

			Intersection = intersectSphere(greyphere.getCenter(), org, dir, greyphere.getRadius(), t);
			if (Intersection)
			{
				t_arr.push_back(t);
				color_arr.push_back(greyphere.getMyColor());
			}

			if (t_arr.size() == 0)
			{
				image[x][y].x = 1.0;
				image[x][y].y = 1.0;
				image[x][y].z = 1.0;
			}
			else
			{
				min_t = 1000.0;
				whichone = 0;
				for (i = 0; i < t_arr.size(); i++)
				{
					if (t_arr[i] < min_t) { whichone = i; min_t = t_arr[i]; }
				}
				image[x][y].x = color_arr[whichone].x;
				image[x][y].y = color_arr[whichone].y;
				image[x][y].z = color_arr[whichone].z;
			}
		}
	}
```

 
## Redesign data class
 
Before your code becomes too complex this is a good opportunity to refactor your ray casting code
so that it is capable of rendering any primitive e.g. planes and triangles.

Step 1: Create a base Shape class which should contain a virtual method for the intersection and
then inherit Sphere from this class. Test that your code still runs correctly after the refactoring.

Step 2: Add a Plane class that also inherits from your Shape class. The plane can be defined by a
point on the plane and the normal to the plane. Add the intersect method using the ray-plane
intersection method (see lecture slides for more details). Add a floor plane instead of the floor
Sphere and test your code. Your scene may look very similar to before but actually the previous floor
Sphere had a very slight curve whereas your floor will now be flat.

Note: you will need to experiment with the field of view angle to effectively zoom out and see more
of the floor.

![Plane picture](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202/Readme%20Pictures/Plane.jpg)

## Add triangle class

Add a Triangle class that also inherits from your Shape class. A triangle can be defined by 3
vertices. Add the intersect method using the ray-triangle intersection method (see lecture slide for
more details). Add a triangle to a scene and test that it appears correctly.

![Triangle picture](https://github.coventry.ac.uk/ac7020/322COM_TeachingMaterial/blob/master/Session%202/Readme%20Pictures/Triangle.jpg)

Triangle with vertices: (0, 1, -2), (-1.9, -1, -2), (1.6, -0.5, -2)

## Rendering complex shape

This section is advanced level so it is optional. 
To render more complex shapes e.g. teapot you will need to be able to load a mesh from a file and
render each of its triangles. A simple mesh loader (OBJloader.h) written in C++ can be downloaded from Week 2 folder.
Note this code is limited to only loading meshes of file type OBJ. 
For using OBJloader.h, you also need to add Vertex.h and GLEW libary header (glew.h) into the project.

The mesh loader handle the import of OBJ file is inside the header file : OBJloader.h.
This is a head file only library. It is a simplified version of Bly7 OBJ Loader library ( https://github.com/Bly7/OBJ-Loader  ).

Examples of a house and a pig OBJ files can also be downloaded from week 2.

## Example of mesh class

This section is advanced level so it is optional.

An example of abstract shape class
 ```C++
#pragma once
#include "glm/glm/glm.hpp"

using namespace glm;

class shape
{
private:
	vec3 center;
	vec3 diffuseColor; 
	vec3 diffuseItensity; 

public:
	shape();
	virtual bool intersection(vec3 rayDirection, vec3 rayOrigin, float& t, vec3& IntPt, vec3& normVec);
	
	//for future ray tracing
	virtual void ComputeColor(const float ambientIntensity, const vec3 IntPt, const vec3 lightPt, const vec3 rayDirection, float& ColValue);
	~shape();
	vec3 position;
	vec3 mcolor;
};
```

You need to write your own CPP file for shape class

An example of mesh/triangle class
 ```C++
#pragma once
#include "shape.h"
#include "glm/glm/glm.hpp"

class triangle : public shape
{
private:
	vec3 vertex0, vertex1, vertex2;
	vec3 norm0, norm1, norm2;

public:
	triangle(vec3 pos, vec3 v0, vec3 v1, vec3 v2, vec3 color);
	triangle(vec3 pos, vec3 v0, vec3 v1, vec3 v2,
		vec3 n0, vec3 n1, vec3 n2,
		vec3 color) :
		vertex0(v0), vertex1(v1), vertex2(v2), norm0(n0), norm1(n1), norm2(n2)
	{
		mcolor = color;
	};

	bool intersection(vec3 rayDirection, vec3 rayOrigin, float& t, vec3 &IntPt, vec3& normVec)override;
	void ComputeColor(const float ambientIntensity, const vec3 IntPt, const vec3 lightPt, const vec3 rayDirection, const vec3 tNormvec, float& ColValue);
	~triangle();
};
```

An example of triangle interection implementation (Moller-Trumbore method).
The method is explained in https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection

```C++
bool triangle::intersection(vec3 rayDirection, vec3 rayOrigin, float &t, vec3 &IntPt, vec3 &normVec)
{
	//Moller-Trumbore

	vec3 v0v1 = vertex1 - vertex0;
	vec3 v0v2 = vertex2 - vertex0;

	float u = (dot((rayOrigin - vertex0), (cross(rayDirection, v0v2)))) / dot(v0v1, cross(rayDirection, v0v2));
	float v = (dot(rayDirection, cross(rayOrigin - vertex0, v0v1))) / dot(v0v1, cross(rayDirection, v0v2));

	float w = 1 - u - v;

	if (u < 0 || u > 1)
		return false;
	else if (v < 0 || (u + v) > 1)
		return false;
	else
	{
        //compute intersection point, t and normal vector
		//add your codes here
		return true;
	}
}
```

## Add screen display using SDL2 Library

If you want to output rendering result directly to the screen, you should use SDL2.
Simple DirectMedia Layer (SDL2) is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D. 

SDL officially supports Windows, Mac OS X, Linux, iOS, and Android.

An example of screen display is provide in _"SDLPixel.cpp"_. The example code only display a white screen. You need to add ray casting/tracing codes.

The function PutPixel32_nolock ouput color into computer screen. You need to create a Uint32 color object using convertColour() function.

If you use empty project as startup option in Visual Studio, you must choose the Console option. 
It is Project Property->Linker->System->SubSystem. SubSystem should set to Console (/SUBSYSTEM:CONSOLE)

