// The ESA/ESO/NASA FITS Liberator - http://code.google.com/p/fitsliberator
//
// Copyright (c) 2004-2010, ESA/ESO/NASA.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the names of the European Space Agency (ESA), the European 
//       Southern Observatory (ESO) and the National Aeronautics and Space 
//       Administration (NASA) nor the names of its contributors may be used to
//       endorse or promote products derived from this software without specific
//       prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL ESA/ESO/NASA BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// =============================================================================
//
// The ESA/ESO/NASA FITS Liberator uses NASA's CFITSIO library, libtiff, 
// TinyXML, Boost C++ Libraries, Object Access Library and Intel Threading 
// Building Blocks.
//
// =============================================================================
//
// Project Executive:
//   Lars Lindberg Christensen
//
// Technical Project Manager:
//   Lars Holm Nielsen
//
// Developers:
//   Kaspar Kirstein Nielsen & Teis Johansen
// 
// Technical, scientific support and testing: 
//   Robert Hurt
//   Davide De Martin
//
// =============================================================================
#include "Image.h"

using namespace FitsLiberator;

Color::Color() {
    red = blue = green = 0;
}

Color::Color(const Color& rhs) {
    red = rhs.red;
    green = rhs.green;
    blue = rhs.blue;
}

Color::Color( Byte red, Byte green, Byte blue ) {
	this->red = red;
	this->blue = blue;
	this->green = green;
}

Color& Color::operator=(const Color& rhs) {
    red = rhs.red;
    green = rhs.green;
    blue = rhs.blue;
    return *this;
}

Color::operator UInt() const {
    return red | ((UInt)green) << 8 | ((UInt)blue) << 16;
}

FitsLiberator::Size::Size() {
	Size(0, 0);
}

FitsLiberator::Size::Size(UInt width, UInt height) {
    this->width = width;
    this->height = height;
}

UInt FitsLiberator::Size::getArea()
{
	return (this->width) * (this->height);
}

FitsLiberator::Rectangle::Rectangle() {
	Rectangle(0, 0, 0, 0);
}

FitsLiberator::Rectangle::Rectangle(Int left, Int top, Int right, Int bottom) {
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

Bool FitsLiberator::Rectangle::hasArea()
{
	if ((this->bottom - this->top) * (this->left - this->right) != 0)
	{
		return true;
	}
	return false;
}

UInt FitsLiberator::Rectangle::getHeight() const
{
	return (UInt)FitsLiberator::Engine::FitsMath::absolute(this->bottom - this->top);
}

UInt FitsLiberator::Rectangle::getWidth() const
{
	return (UInt)FitsLiberator::Engine::FitsMath::absolute(this->right - this->left);
}

UInt FitsLiberator::Rectangle::getArea() const
{
	return getWidth() * getHeight();
}

FitsLiberator::Point::Point() {
	Point(0, 0);
}

FitsLiberator::Point::Point(Int x, Int y) {
	this->x = x;
	this->y = y;
}

Image::Image() {
	pixels = NULL;
}

FitsLiberator::Size Image::getSize() {
    return Size(this->size.width, this->size.height);
}

Color* Image::getPixels() {
    Color* p = this->pixels;
    return p;
}