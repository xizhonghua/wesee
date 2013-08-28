/*
 * GrabCut implementation source code 
 * by Justin Talbot, jtalbot@stanford.edu
 * Placed in the Public Domain, 2010
 * 
 */

#include "Color.h"
#include "GL/gl.h"
#include "opencv/cv.h"

Real distance2( const Color& c1, const Color& c2 )
{
	return ((c1.r-c2.r)*(c1.r-c2.r)+(c1.g-c2.g)*(c1.g-c2.g)+(c1.b-c2.b)*(c1.b-c2.b));
}

void display(Image<Color>& image)
{
//#ifdef USE_DOUBLE
//	glDrawPixels( image.width(), image.height(), GL_RGB, GL_DOUBLE, image.ptr() );
//#else
	glDrawPixels( image.width(), image.height(), GL_RGB, GL_FLOAT, image.ptr() );
//#endif
}

void display(Image<Real>& image, GLenum format)
{
//#ifdef USE_DOUBLE
//	glDrawPixels( image.width(), image.height(), format, GL_DOUBLE, image.ptr() );
//#else
	glDrawPixels( image.width(), image.height(), format, GL_FLOAT, image.ptr() );
//#endif
}


Image<Color>* load( std::string file_name )
{
	if( file_name.find( ".pgm" ) != std::string::npos )
	{
		return loadFromPGM( file_name );
	}

	else if( file_name.find( ".ppm" ) != std::string::npos )
	{
		return loadFromPPM( file_name );
	}

	else
	{
		return NULL;
	}
}

Image<Color>* loadForOCV(std::string file_name, const int long_edge)
{
	Mat im = MatHelper::read_image(file_name, long_edge);

	if(!im.data){
		cout<<"filed to load image "<<file_name<<endl;
		return NULL;
	}

	Image<Color>* image = new Image<Color>(im.cols, im.rows);

	for(int i=0; i<im.rows; i++)
	    for(int j=0; j<im.cols; j++)
	    {
	    	double b = im.at<cv::Vec3b>(i,j)[0];
	    	double g = im.at<cv::Vec3b>(i,j)[1];
	    	double r = im.at<cv::Vec3b>(i,j)[2];

	    	Real R, G, B;
	    	R = (Real)((unsigned char)r)/255;
	    	G = (Real)((unsigned char)g)/255;
	    	B = (Real)((unsigned char)b)/255;

	    	(*image)(j, (im.rows-1)-i) = Color(R,G,B);
	    }

	return image;

}

Image<Color>* loadFromPGM( std::string file_name ) 
{
	std::ifstream inPGM( file_name.c_str(), std::ios::in | std::ios::binary );
	if( !inPGM.is_open() ) return NULL;

	char version[2];
	inPGM.read(version, 2);

	char line[1000];
	char t;

	if( version[0] == 'P' && version[1] == '5' ) {

		std::string comment; 
	    
		while( inPGM.peek() < 48 || inPGM.peek() > 57 )
		{
			if( inPGM.peek() == '#' )
				inPGM.getline( line, 1000 );
			else
				inPGM.read( &t, 1 );
		}
	    
		unsigned int width, height, maximum;

		inPGM >> width >> height;
		inPGM >> maximum;

		Image<Color>* image = new Image<Color>(width, height);

		if (image)
		{
			if (inPGM.peek() == 13) inPGM.read( &t, 1 );
			if (inPGM.peek() == 10) inPGM.read( &t, 1 );

			for (unsigned int i = 0; i < height; ++i) 
			{
				for (unsigned int j = 0; j < width; ++j) 
				{
					inPGM.read( &t, 1 );
					Real c = (Real)((unsigned char)t)/maximum;
					(*image)(j, (height-1)-i) = Color(c,c,c);
				}      
			}
		}

		inPGM.close();

		return image;
	}
	else
	{
		return NULL;
	}
}

Image<Color>* loadFromPPM( std::string file_name ) 
{
	std::ifstream inPPM( file_name.c_str(), std::ios::in | std::ios::binary );
	if( !inPPM.is_open() ) return NULL;

	char version[2];
	inPPM.read(version, 2);

	char line[1000];
	char t;

	if( version[0] == 'P' && version[1] == '6' ) {

		std::string comment; 
	    
		while( inPPM.peek() < 48 || inPPM.peek() > 57 )
		{
			if( inPPM.peek() == '#' )
				inPPM.getline( line, 1000 );
			else
				inPPM.read( &t, 1 );
		}
	    
		unsigned int width, height, maximum;

		inPPM >> width >> height;
		inPPM >> maximum;

		Image<Color>* image = new Image<Color>(width, height);

		if (image)
		{
			if (inPPM.peek() == 13) inPPM.read( &t, 1 );
			if (inPPM.peek() == 10) inPPM.read( &t, 1 );

			for (unsigned int i = 0; i < height; ++i)
			{
				for (unsigned int j = 0; j < width; ++j)
				{
					char r, g, b;

					inPPM.read( &r, 1 );
					inPPM.read( &g, 1 );
					inPPM.read( &b, 1 );
				
					Real R, G, B;
					R = (Real)((unsigned char)r)/maximum;
					G = (Real)((unsigned char)g)/maximum;
					B = (Real)((unsigned char)b)/maximum;

					(*image)(j, (height-1)-i) = Color(R,G,B);
				}
			}
		}

		inPPM.close();

		return image;
	}

	return 0;
}
