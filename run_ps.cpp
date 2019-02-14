#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <armadillo>
#include "lodepng.h"

using namespace std;
using namespace arma;

// http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html
void split(const string& str, vector<string>& cont, char delim = ' ')
{
    stringstream ss(str);
    string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}


int main(int argc, char *argv[]) {
	// ----------------------------------------
	// Get light positions from file
	// ----------------------------------------
	int numLights;
	arma::mat lights;
	string line;
	ifstream myfile ("light_positions.txt");
	int currentLight = 0;
	if (myfile.is_open()) {
		getline(myfile,line);
		numLights = stoi(line);
		lights.set_size(numLights,3);

		while (getline(myfile,line)) {
			int start = line.find('(');
			int end = line.find(')');
	        vector<string> words;
    	    split(line.substr(start+1,end-1), words, ' ');
			
    	    for (int p=0; p<3; p++) lights(currentLight, p) = stof(words[p]);

			currentLight++;
			if (currentLight == numLights) break;
		}
		myfile.close();
	} else cout << "Unable to open file"; 
	// lights.print("Lights:");

	// ----------------------------------------
	// Read mask
	// ----------------------------------------
	const char* filename = argc > 1 ? argv[1] : "sphere";
	string pathStr = "psmImages/" + string(filename) + "/" + string(filename);
	string maskFilename = pathStr + ".mask.png";
	vector<unsigned char> mask; //the raw pixels
	unsigned width, height;
	unsigned error = lodepng::decode(mask, width, height, maskFilename);
	//if there's an error, display it
	if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;

	// ----------------------------------------
	// Read images
	// ----------------------------------------
	vector<vector<unsigned char>> images;
	images.resize(numLights);

	vector<unsigned char> temp;


	for (int i=0; i<numLights; i++) {
		string str = pathStr + "." + to_string(i) + ".png";
		error = lodepng::decode(images[i], width, height, str.c_str());
		//if there's an error, display it
		if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
	}

	// ----------------------------------------
	// Compute surface normals
	// ----------------------------------------
	// p = number of lights
	// Initialize T, a height x width x p matrix, whose (h, w, :) holds the intesities at (h, w) for all p different lights
	vector<vector<arma::vec>> T;
	T.resize(height);
	for (int i=0; i<height; i++) {
		T[i].resize(width);
		for (int j=0; j<width; j++) {
			T[i][j].set_size(numLights);
		}
	}

	float r, g, b, len;
	for (int i=0; i<numLights; i++) {
		for (int h=0; h<height; h++) {
			for (int w=0; w<width; w++) {
				// if mask is black, skip
				if (mask[h*width*4+w*4+0] == 0) continue;

				// get rgb values
				r = images[i][h*width*4+w*4+0];
				g = images[i][h*width*4+w*4+1];
				b = images[i][h*width*4+w*4+2];
				// normalize
				T[h][w](i) = sqrt(r*r + g*g + b*b);

				images[i][h*width*4+w*4+0] = T[h][w](i)*255/442;
				images[i][h*width*4+w*4+1] = T[h][w](i)*255/442;
				images[i][h*width*4+w*4+2] = T[h][w](i)*255/442;

			}
		}
	}

	// Initilize N, a height x width x 3 matrix, whose (h, w, :) holds the surface normal at (h,w)
	vector<vector<arma::vec>> N;
	N.resize(height);
	for (int i=0; i<height; i++) {
		N[i].resize(width);
		for (int j=0; j<width; j++) {
			N[i][j].set_size(3);
			N[i][j] << 0 << 0 << 0;
		}
	}

	float kd;
	arma::vec n = arma::zeros<arma::vec>(3);
	arma::vec inten(numLights);
	for (int h=0; h<height; h++) {
		for (int w=0; w<width; w++) {
			n << 0 << 0 << 0;
			// if mask is black, skip
			if (mask[h*width*4+w*4+0] == 0) continue;

			// Intesities
			inten = resize(T[h][w], numLights, 1);

 			// Solve surface normals
			n = arma::solve(lights, inten);

			// get the albedo
			kd = norm(n);

			if (kd != 0) {
				// Normalize n
				n /= kd;
			} 

			N[h][w] = n;
		}
	}

	//generate some image
	vector<unsigned char> nImage;
	nImage.resize(width * height * 4);
	for(unsigned h = 0; h < height; h++) {
		for(unsigned w = 0; w < width; w++) {
			if (N[h][w](0) > 0)
				nImage[4 * width * h + 4 * w + 0] = N[h][w](0)*255;
			else 
				nImage[4 * width * h + 4 * w + 0] = 0;

			if (N[h][w](1) > 0)
				nImage[4 * width * h + 4 * w + 1] = N[h][w](1)*255;
			else
				nImage[4 * width * h + 4 * w + 1] = 0;

			if (N[h][w](2) > 0)
				nImage[4 * width * h + 4 * w + 2] = N[h][w](2)*255;
			else
				nImage[4 * width * h + 4 * w + 2] = 0;

			nImage[4 * width * h + 4 * w + 3] = 255;
		}
	}

	error = lodepng::encode("results/" + string(filename) + ".png", nImage, width, height);
	//if there's an error, display it
	if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
	
	return 0;
}