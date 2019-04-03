// Algorithm based on Woodham, R.J. 1980. Photometric method for determining surface orientation from multiple images. Optical Engineerings 19, I, 139-144.
// Code adapted from Matlab implementation at https://github.com/xiumingzhang/photometric-stereo

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

void split(const string& str, vector<string>& cont, char delim);
bool readLightsFile(mat& lights, string filename);
void drawNormalMap(vector<vector<vec>>N, int height, int width, string filename);
void computeSurfaceNormals(vector<vector<vec>>& N, mat& lights, vector<vector<unsigned char>>& images, vector<unsigned char>& mask, int height, int width);

int main(int argc, char *argv[]) {
	const char* filename = argc > 1 ? argv[1] : "antoninuspious";
	string pathStr = "psmImages/" + string(filename) + "/" + string(filename);

	// ----------------------------------------
	// Get light positions from file
	// ----------------------------------------
	mat lights;
	if (!readLightsFile(lights, "psmImages/light_positions.txt")) return -1;
	int numLights = lights.n_rows;

	// ----------------------------------------
	// Read mask
	// ----------------------------------------
	vector<unsigned char> mask; //the raw pixels
	unsigned width, height;
	unsigned error = lodepng::decode(mask, width, height, pathStr + ".mask.png");
	//if there's an error, display it
	if(error) {
		cout << "lodepng::decoder error mask" << error << ": " << lodepng_error_text(error) << endl;
		return -1;
	}

	// ----------------------------------------
	// Read images
	// ----------------------------------------
	vector<vector<unsigned char>> images;
	images.resize(numLights);
	for (int i=0; i<numLights; i++) {
		error = lodepng::decode(images[i], width, height, pathStr + "." + to_string(i) + ".png");
		//if there's an error, display it
		if(error) {
			cout << pathStr + "." + to_string(i) + ".png" << endl;
			cout << "lodepng::decoder error " << i << " " << error << ": " << lodepng_error_text(error) << endl;
			return -1;
		}
	}

	// ----------------------------------------
	// Compute surface normals
	// ----------------------------------------
	// Initilize N, a height x width x 3 matrix, whose (h, w, :) holds the surface normal at (h,w)
	vector<vector<vec>> N;
	N.resize(height);
	for (int i=0; i<height; i++) {
		N[i].resize(width);
		for (int j=0; j<width; j++) {
			N[i][j].set_size(3);
			N[i][j] << 0 << 0 << 0;
		}
	}

	computeSurfaceNormals(N, lights, images, mask, height, width);
	drawNormalMap(N, height, width, filename);

	// free the images as we won't be using them again
	images.clear();
	images.shrink_to_fit();

	return 0;

	// ----------------------------------------
	// Compute depth 
	// ----------------------------------------
	// get indices of all pixels that represent a point on the object, i.e. are not the background
	vector<int> objH, objW;
	for (int h=0; h<height; h++) {
		for (int w=0; w<width; w++) {
			if (mask[h*width*4+w*4+0] == 0) continue;
			objH.push_back(h);
			objW.push_back(w);
		}
	}

	int numPixels = objH.size();
	// mapping from (h, w) pixel coordinates to index in objH and objW
	mat fullToObject(height, width); 
	fullToObject.zeros();
	for (int i=0; i<numPixels; i++) {
		fullToObject(objH[i], objW[i]) = i;
	}

	sp_mat M(2*numPixels, numPixels); // why are these so big?
	mat u(2*numPixels, 1);

	// assemble M and u
	vector<int> failedRows;
	int h, w, rowIndex, vertNIndex, horizNI;
	float nx, ny, nz;
	for (int i=0; i<numPixels; i++) {
		// position in 2D image
		h = objH[i];
		w = objW[i];
		// surface normal
		nx = N[h][w](0);
		ny = N[h][w](1);
		nz = N[h][w](2);
		// first row - vertical neighbors
		rowIndex = (i)*2; // what is this?
		// filter our potentially harmful points
		if (mask[(h+1)*width*4+w*4+0] != 0) { // check if down neighbor is in bound
			vertNIndex = fullToObject(h+1, w);
			u(rowIndex) = -ny;
			M(rowIndex, i) = -nz;
			M(rowIndex, vertNIndex) = nz;
		} else if (mask[(h+1)*width*4+w*4+0] != 0) { // check if up neighbor is in bound
			vertNIndex = fullToObject(h-1, w);
			u(rowIndex) = ny;
			M(rowIndex, i) = -nz;
			M(rowIndex, vertNIndex) = nz;
		} else { // no vertical neighbors
			failedRows.push_back(rowIndex);
		}
		// second row - horizontal neighbors
		if (mask[h*width*4+(w+1)*4+0] != 0) { // check if right neighbor is in bound
			vertNIndex = fullToObject(h, w+1);
			u(rowIndex) = -nx;
			M(rowIndex, i) = -nz;
			M(rowIndex, vertNIndex) = nz;
		} else if (mask[(h-1)*width*4+(w-1)*4+0] != 0) { // check if left neighbor is in bound
			vertNIndex = fullToObject(h, w-1);
			u(rowIndex) = nx;
			M(rowIndex, i) = -nz;
			M(rowIndex, vertNIndex) = nz;
		} else { // no horizontal
			failedRows.push_back(rowIndex);
		}
	}

	// remove all-zero rows
	int numShed = 0;
	for (int i=0; i<failedRows.size(); i++) {
		M.shed_row(failedRows[i]-numShed);
		u.shed_row(failedRows[i]-numShed);
		numShed++;
	}

	mat z = spsolve(M.t() * M, M.t() * u);

	// outliers due to singularity
	// outlier_ind = abs(zscore(z))>10;
	// z_min = min(z(~outlier_ind));
	// z_max = max(z(~outlier_ind));

	// reassemble z back to 2D
	// Z = double(mask);
	// for idx = 1:no_pix
	//     % Position in 2D image
	//     h = obj_h(idx);
	//     w = obj_w(idx);
	//     % Rescale
	//     Z(h, w) = (z(idx)-z_min)/(z_max-z_min)*255;
	// end	

	return 0;
}

// utility function to split strings
// http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html
void split(const string& str, vector<string>& cont, char delim = ' ')
{
    stringstream ss(str);
    string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

bool readLightsFile(mat& lights, string filename) {
	int numLights;
	string line;
	ifstream myfile (filename);
	int currentLight = 0;
	if (myfile.is_open()) {
		getline(myfile,line);
		numLights = stoi(line);
		lights.set_size(numLights,3);

		while (getline(myfile,line)) {
			vector<string> words;
    	    split(line, words, ' ');

    	    for (int p=0; p<3; p++) lights(currentLight, p) = stof(words[p]);

			currentLight++;
			if (currentLight == numLights) break;
		}
		myfile.close();
		return true;
	} else { 
		cout << "Unable to open " << filename << endl; 
		return false;
	}
}

void computeSurfaceNormals(vector<vector<vec>>& N, mat& lights, vector<vector<unsigned char>>& images, vector<unsigned char>& mask, int height, int width) {
	int numLights = images.size();

	// Initialize T, a height x width x p matrix, whose (h, w, :) holds the intesities at (h, w) for all p different lights
	vector<vector<vec>> T;
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

	float kd;
	vec n = zeros<vec>(3);
	vec inten(numLights);
	for (int h=0; h<height; h++) {
		for (int w=0; w<width; w++) {
			n << 0 << 0 << 0;
			// if mask is black, skip
			if (mask[h*width*4+w*4+0] == 0) continue;

			// Intesities
			inten = resize(T[h][w], numLights, 1);

 			// Solve surface normals
			n = solve(lights.t() * lights, lights.t() * inten);

			// get the albedo
			kd = norm(n);

			if (kd != 0) {
				// Normalize n
				n /= kd;
			} 

			N[h][w] = n;
		}
	}
}

void drawNormalMap(vector<vector<vec>> N, int height, int width, string filename) {
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

	unsigned error = lodepng::encode("results/" + string(filename) + ".png", nImage, width, height);
	//if there's an error, display it
	if(error) cout << "lodepng::encoder error " << error << ": " << lodepng_error_text(error) << endl;
	
}