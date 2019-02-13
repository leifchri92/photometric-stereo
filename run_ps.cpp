#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <armadillo>
#include "lodepng.h"

using namespace std;

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
	int numLights = 12;
	// float lights[numLights][3];
	arma::mat lights(numLights,3);
	string line;
	ifstream myfile ("light_positions.txt");
	int currentLight = 0;
	if (myfile.is_open()) {
		while (getline(myfile,line)) {
			int start = line.find('(');
			int end = line.find(')');
	        vector<string> words;
    	    split(line.substr(start+1,end-1), words, ',');
			
    	    // for (int p=0; p<3; p++) lights[currentLight][p] = stof(words[p]);
    	    for (int p=0; p<3; p++) lights(currentLight, p) = stof(words[p]);

			currentLight++;
			if (currentLight == numLights) break;
		}
		myfile.close();
	} else cout << "Unable to open file"; 

	lights.print("Lights:");

	return 0;

	// ----------------------------------------
	// Read mask
	// ----------------------------------------
	const char* filename = argc > 1 ? argv[1] : "img/sphere/sphere-mask.png";
	vector<unsigned char> mask; //the raw pixels
	unsigned width, height;
	unsigned error = lodepng::decode(mask, width, height, filename);
	//if there's an error, display it
	if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;

	// ----------------------------------------
	// Read images
	// ----------------------------------------
	vector<vector<unsigned char>> images;
	images.resize(numLights);
	for (int i=0; i<numLights; i++) {
		images[i].resize(width * height * 4);
		string str = "img/sphere/sphere" + to_string(i) + ".png";
		error = lodepng::decode(images[i], width, height, str.c_str());
		//if there's an error, display it
		if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
	}

	// ----------------------------------------
	// Compute surface normals
	// ----------------------------------------
	// p = number of lights
	// Initialize T, a height x width x p matrix, whose (h, w, :) holds the intesities at (h, w) for all p different lights
	vector<vector<vector<float>>> T;
	T.resize(height);
	for (int i=0; i<height; i++) {
		T[i].resize(width);
		for (int j=0; j<width; j++) {
			T[i][j].resize(numLights);
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
				T[h][w][i] = sqrt(r*r + g*g + b*b);
			}
		}
	}

	// Initilize N, a height x width x 3 matrix, whose (h, w, :) holds the surface normal at (h,w)
	vector<vector<vector<float>>> N;
	N.resize(height);
	for (int i=0; i<height; i++) {
		N[i].resize(width);
		for (int j=0; j<width; j++) {
			N[i][j].resize(3);
		}
	}

	float nNorm;
	arma::vec n = arma::zeros<arma::vec>(3);

	n << 1 << 2 << 3;
	n.print();

	cout << arma::norm(n) << endl;
	cout << sqrt(n(0)*n(0) + n(1)*n(1) + n(2)*n(2)) << endl;

	return 0;
	for (int h=0; h<height; h++) {
		for (int w=0; w<width; w++) {
			// if mask is black, skip
			if (mask[h*width*4+w*4+0] == 0) continue;

			//  i = reshape(T(h, w, :), [p, 1]);
			// T[h][w]
 			// Solve surface normals
 			// ' is the transpose
 			// .' is the unconjugated copmlex transpose
			// n = (L.'*L)\(L.'*i);
			// inv(L.t()*L)*L.t()*i

			nNorm = norm(n);

			if (nNorm != 0) {

			} else {
				n << 0 << 0 << 0;
			}

			N[h][w][0] = n[0];
			N[h][w][1] = n[1];
			N[h][w][2] = n[2];
		}
	}


	return 0;
}