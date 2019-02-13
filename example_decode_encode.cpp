/*
LodePNG Examples

Copyright (c) 2005-2012 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "lodepng.h"
#include <iostream>

int main(int argc, char *argv[]) {
	const char* filename = argc > 1 ? argv[1] : "img/color_test.png";
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	//decode
	unsigned error = lodepng::decode(image, width, height, filename);
	//if there's an error, display it
	if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//Encode the image
	error = lodepng::encode("test.png", image, width, height);
	//if there's an error, display it
	if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;

	return 1;
}