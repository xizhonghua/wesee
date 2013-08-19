/*
 * GrabCut implementation source code 
 * by Justin Talbot, jtalbot@stanford.edu
 * Placed in the Public Domain, 2010
 * 
 */

#ifndef GRAB_CUT_H
#define GRAB_CUT_H

#include "Image.h"
#include "Color.h"
#include "GMM.h"

#include "graph.h"

typedef Graph<double, double, double> GraphType;

class GrabCut
{
public:

	GrabCut( Image<Color> *image );

	~GrabCut();

	// Initialize Trimap, inside rectangle is TrimapUnknown, outside is TrimapBackground
	void initialize(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	// Edit Trimap
	void setTrimap(int x1, int y1, int x2, int y2, const TrimapValue& t);
	
	void fitGMMs();

	// Run Grabcut refinement on the hard segmentation
	void refine();
	int refineOnce();	// returns the number of pixels that have changed from foreground to background or vice versa

	//OpenGL display routine
	void display( int t );
	void overlayAlpha();

private:

	unsigned int m_w, m_h;				// All the following Image<*> variables will be the same width and height.
										// Store them here so we don't have to keep asking for them.
	Image<Color> *m_image;
	Image<TrimapValue> *m_trimap;
	Image<unsigned int> *m_GMMcomponent;
	Image<SegmentationValue> *m_hardSegmentation;

	Image<Real> *m_softSegmentation;	// Not yet implemented (this would be interpreted as alpha)

	GMM *m_backgroundGMM, *m_foregroundGMM;

	int updateHardSegmentation();		// Update hard segmentation after running GraphCut, 
										// Returns the number of pixels that have changed from foreground to background or vice versa.

	// Variables used in formulas from the paper.
	Real m_lambda;		// lambda = 50. This value was suggested the GrabCut paper.
	Real m_beta;		// beta = 1 / ( 2 * average of the squared color distances between all pairs of neighboring pixels (8-neighborhood) )
	Real m_L;			// L = a large value to force a pixel to be foreground or background

	void computeBeta();
	void computeL();

	// Precomputed N-link weights
	Image<NLinks> *m_NLinks;

	void computeNLinks();
	Real computeNLink(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	// Graph for Graphcut
	GraphType *m_graph;
	Image<GraphType::node_id> *m_nodes;

	void initGraph();	// builds the graph for GraphCut

	// Images of various variables that can be displayed for debugging.
	Image<Real> *m_NLinksImage;
	Image<Color> *m_TLinksImage;
	Image<Color> *m_GMMImage;
	Image<Real> *m_AlphaImage;

	void buildImages();
};


#endif //GRAB_CUT_H
