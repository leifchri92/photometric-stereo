#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include "lodepng.h"

// takes two arguments:
// 		1. name of original
// 		2. extension indicating defenses

int main(int argc, char *argv[]) {
	const char* in1 = argc > 1 ? argv[1] : "";
	std::string filename = "./results/" + std::string(in1);

	const char* in2 = argc > 2 ? argv[2] : "";
	std::string ext = std::string(in2);
	
	std::string filename_original = filename + std::string(".normals.png");
	std::vector<unsigned char> image_original; //the raw pixels
	unsigned width, height, error;
	int image_size;

	// Read the original

	//decode
	error = lodepng::decode(image_original, width, height, filename_original.c_str());
	//if there's an error, display it
	if(error) {
		std::cout << "decoder error " << filename_original << error << ": " << lodepng_error_text(error) << std::endl;
		return -1;
	}
	image_size = width*height*4;

	// Read the mask

	std::string filename_mask = filename + std::string(".mask.png");
	std::vector<unsigned char> image_mask; //the raw pixels

	error = lodepng::decode(image_mask, width, height, filename_mask.c_str());
	if(error) {
		std::cout << "decoder error " << filename_mask << error << ": " << lodepng_error_text(error) << std::endl;
		return -1;
	}

	// Read the reconstruction
	std::vector<unsigned char> image_comp;
	std::string filename_comp = filename + ext +".png";

	error = lodepng::decode(image_comp, width, height, filename_comp.c_str());
	if(error) {
		std::cout << "decoder error " << filename_comp << error << ": " << lodepng_error_text(error) << std::endl;
		return -1;
	}

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA...

	// Compare
	std::vector<unsigned char> image_heatmap;
	image_heatmap.resize(image_size);

	std::ofstream csv_file;
	csv_file.open ("ps_comp-" + std::string(in1) + ext + ".csv");

	int counter = 0;
	for (int i=0; i<image_size; i+=4) {
		counter++;

		double dist = 0;
		for (int j=0; j<3; j++) {
			dist += ( ( int(image_original[i+j]) - int(image_comp[i+j]) ) * ( int(image_original[i+j]) - int(image_comp[i+j]) ) );
			image_heatmap[i+j] = abs(int(image_original[i+j]) - int(image_comp[i+j]));
		}
		dist = sqrt(dist);

		// std::cout<< int(image_mask[i]);
		// break;
		if (int(image_mask[i]) == 0) {
			dist = -1;
			image_heatmap[i+3] = 0;
		} else {
			image_heatmap[i+3] = 255;
		}

		csv_file << dist;

		if (counter != width) {
			csv_file << ',';
		} else {
			csv_file << '\n';
			counter = 0;
		}
	}
	csv_file.close();

	// Write difference

	std::string out_filename = filename + ext + std::string(".comp.png");

	//encode
	error = lodepng::encode(out_filename, image_heatmap, width, height);

	//if there's an error, display it
	if(error) {
		std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
		return -1;
	}

	return 0;
}