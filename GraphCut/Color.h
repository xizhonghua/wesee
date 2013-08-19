/*
 * GrabCut implementation source code 
 * by Justin Talbot, jtalbot@stanford.edu
 * Placed in the Public Domain, 2010
 * 
 */

#ifndef COLOR_H
#define COLOR_H

#include "Global.h"
#include "Image.h"

#include "GL/glut.h"
//class Color;
//class Image<Color>;

class Color {

public:

	Color() : r(0), g(0), b(0) {}
	Color(Real _r, Real _g, Real _b) : r(_r), g(_g), b(_b) {}

	Real r, g, b;
};

typedef Image<Color> IMGColor;

// Compute squared distance between two colors
Real distance2( const Color& c1, const Color& c2 );

// Display function for Color images
void display(Image<Color>& image);


void display(Image<Real>& image, GLenum format=GL_LUMINANCE);

// Loading functions for Color images
Image<Color>* load( std::string file_name );
Image<Color>* loadFromPGM( std::string file_name );
Image<Color>* loadFromPPM( std::string file_name );

Image<Color>* loadForOCV(std::string file_name);

#endif //COLOR_H
