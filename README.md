# Simple Video Steganography using LSB encoding

Built off of the Simple Image Steganography using LSB encoding program originally created by Dr. Miguel Alonso Jr. Tested with .txt encryption onto .mp4 and .mov files.

Note: Any .txt file can be used for encryption purposes, but I've left "pom.txt" as an example text file for quick testing purposes.

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
