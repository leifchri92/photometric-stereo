# photometric-stereo

An implementation of classic photometric stereo ala Woodham 1980 [1]. More concise descriptions of the algorithm may be found in more recent computer vision assignments such as those available at [2, 3, 4]. For similar implementations see [5, 6]. Numerous, more robust solutions exist such as [7], solutions which account for more advanced (and realistic) lighting models, unknown light positions, and imperfections introduced in real world situations.

[1] Woodham, R.J. 1980. Photometric method for determining surface orientation from multiple images. Optical Engineerings 19, I, 139-144.  
[2] https://courses.cs.washington.edu/courses/cse455/10wi/projects/project4/  
[3] http://pages.cs.wisc.edu/~csverma/CS766_09/Stereo/stereo.html  
[4] http://www.wisdom.weizmann.ac.il/~vision/visionproj08/From%20normals%20to%20surfaces.pdf  
[5] https://github.com/xiumingzhang/photometric-stereo  
[6] https://github.com/NewProggie/Photometric-Stereo  
[7] https://github.com/yasumat/RobustPhotometricStereo

## Dependencies
sudo apt-get install libarmadillo-dev

## Compilation
g++ -I ./external ./external/lodepng.cpp run_ps.cpp -larmadillo -llapack -lblas
