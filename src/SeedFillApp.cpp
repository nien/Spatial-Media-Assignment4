//
//  SeedFillApp.cpp
//  SeedFill
//
//  Created by Nien Lam on 2/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
//


#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace ci;
using namespace ci::app;
using namespace std;


class SeedFillApp : public AppBasic 
{
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void keyDown( KeyEvent event ); 
	void update();
	void draw();
	void prepareSettings( Settings *settings );
	
 private:	
	// Size of the input images.
	static const int IMG_WIDTH  = 320;
	static const int IMG_HEIGHT = 240;
	static const int IMG_SPACER = 10;
	
	// Keep track of different shapes using values (1-255)
	static const int LABEL_INIT = 40;
	int   mLabel;
	Vec2i mLabelPos[256];  

	Channel mSrcFile1, mSrcFile2, mSrcFile3;
	Channel mSrcImage, mDestImage;
	Channel getChannelFromRawFile( string const &fileName, int width, int height );
	
	void clearDstImage( Channel &dst );
	void processSourceImage( Channel const &src, Channel &dst );

	// Seed fill algorithm.
	bool seedFill( Channel const &src, int x, int y, Channel &dst );
};


void SeedFillApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( IMG_WIDTH * 2 + IMG_SPACER, IMG_HEIGHT );
	settings->setFrameRate( 60.0f );
}

// Creates Cinder channel from raw image file.
// Probably a better way todo this, but this works for now.
Channel SeedFillApp::getChannelFromRawFile( string const &fileName, int width, int height )
{
	ifstream ifs;
	try 
	{
		ifs.exceptions( fstream::failbit );
		ifs.open( getResourcePath( fileName ).c_str(), ios::in );
	}
	catch ( exception &e )
	{
		console() << "ERROR. Can't open file: " << fileName << endl;
		exit( 1 );
	}
	char image[width * height];
	ifs.read( image, width * height );
	ifs.close();
	
	Channel channel = Channel( 320, 240 );
	for ( int ii = 0; ii < width; ii++ )
	{
		for ( int jj = 0; jj < height; jj++ )
		{
			int value = image[jj * width + ii];
			channel.setValue( Vec2i( ii, jj ), value );
		}
	}

	return channel;
}

void SeedFillApp::setup()
{
	mSrcFile1 = getChannelFromRawFile( "flood_fill_test_01.raw", IMG_WIDTH, IMG_HEIGHT );
	mSrcFile2 = getChannelFromRawFile( "flood_fill_test_02.raw", IMG_WIDTH, IMG_HEIGHT );
	mSrcFile3 = getChannelFromRawFile( "flood_fill_test_03.raw", IMG_WIDTH, IMG_HEIGHT );
	mSrcImage = mSrcFile1;
	
	mDestImage = Channel( IMG_WIDTH, IMG_HEIGHT );
	mLabel     = LABEL_INIT;

	gl::enableAlphaBlending( false );
}

void SeedFillApp::mouseDown( MouseEvent event )
{
}

void SeedFillApp::keyDown( KeyEvent event ) 
{
	// Change draw state based on keyboard input.
    if ( event.getChar() == '1' )
	{
		mSrcImage = mSrcFile1;
		clearDstImage( mDestImage );

	}
    else if ( event.getChar() == '2' )
	{	
		mSrcImage = mSrcFile2;
		clearDstImage( mDestImage );
	}
	else if ( event.getChar() == '3' )
	{
		mSrcImage = mSrcFile3;
		clearDstImage( mDestImage );
	}
}

void SeedFillApp::clearDstImage( Channel &dst )
{
	mLabel = LABEL_INIT;

	Channel::Iter iterDst = dst.getIter();
	while ( iterDst.line() ) 
	{
		while ( iterDst.pixel() ) 
		{
			iterDst.v() = 0;
		}
	}
}

void SeedFillApp::update()
{
	processSourceImage( mSrcImage, mDestImage );
}

void SeedFillApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	// Draw source and destination image.
	gl::draw( mSrcImage, Vec2i( 0, 0 ) );
	gl::draw( mDestImage, Vec2i( IMG_WIDTH + IMG_SPACER, 0 ) );
	
	// Draw labels on shapes.
	for ( int ii = LABEL_INIT; ii < mLabel; ii++ )
	{
		stringstream str;
		str << ii - LABEL_INIT;
		gl::drawString( str.str(), mLabelPos[ii] + Vec2i( IMG_WIDTH, 0 ), ColorA( 0, 1, 0 ) );
	}
}

void SeedFillApp::processSourceImage( Channel const &src, Channel &dst )
{
	Channel::ConstIter iterSrc = src.getIter();
	Channel::Iter      iterDst = dst.getIter();
	
	while ( iterSrc.line() && iterDst.line() ) 
	{
		while ( iterSrc.pixel() && iterDst.pixel() ) 
		{
			if ( seedFill( src, iterSrc.x(), iterSrc.y(), dst ) ) 
			{
				// New shape is found. Record current locatin increment label count.
				mLabelPos[mLabel++] = iterSrc.getPos();
			}
		}
	}
}

// Returns true when it encounters a new shape.
bool SeedFillApp::seedFill( Channel const &src, int x, int y, Channel &dst )
{
	// TODO: If outside of image range return false.
	
	// If source image pixel is black.
	if ( src.getValue( Vec2i( x, y ) ) == 0 )
		return false;

	// If destination image pixel is aready set.
	if ( dst.getValue( Vec2i( x, y ) ) > 0 )
		return false;

	// Set pixel to gradient label.
	// TODO: May need additionial check if source image is not pure black and white.
	dst.setValue( Vec2i( x, y ), mLabel );

	// Perform recursion.
	seedFill( src, x, y - 1, dst );
	seedFill( src, x, y + 1, dst );
	seedFill( src, x - 1, y, dst );
	seedFill( src, x + 1, y, dst );
	
	// A new shape found.
	return true;
}


CINDER_APP_BASIC( SeedFillApp, RendererGl )
