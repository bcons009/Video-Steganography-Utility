# Simple Video Steganography using LSB encoding

Built off of the Simple Image Steganography using LSB encoding program originally created by Dr. Miguel Alonso Jr.
Note: "pom.txt" is only an example text file used to test the vsteg program.

## To Compile

~~~~
mkdir build
cd build
cmake ..
cmake --build .
~~~~

## Usage

~~~~
*** Video Steganography by LSB substitution ***

Usage:  
./vsteg [-e] <text file to encode> <source video file> <destination video file>
./vsteg [-d] <encoded video file> <decoded file>

-e : Encode text in video
-d : Decode text from video
~~~~
