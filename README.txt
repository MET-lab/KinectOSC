This guide contains instructions to install the necessary libraries and drivers for the Microsoft Kinect on Mac OS X. This procedure requires git and Homebrew. Skip to step 1 if you have them installed.


0.0) Install git from:  http://git-scm.com/downloads
----------------------------------------------------

0.5) Install Homebrew with the following command:
-------------------------------------------------

ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

- Note: If the first line displayed is an SSL certificate problem, use

ruby -e "$(curl -fsSL --insecure https://raw.github.com/Homebrew/homebrew/go/install)"


1) Install libfreenect using Brew:
----------------------------------

brew install libfreenect
cp /usr/local/Cellar/libfreenect/0.4.0/lib/libfreenect.0.4.0.dylib /usr/local/lib
cp /usr/local/Cellar/libusb/1.0.18/lib/libusb-1.0.0.dylib /usr/local/lib


2) Download OpenNI from http://www.openni.org/openni-sdk/
---------------------------------------------------------

3) Install OpenNI:
------------------

cd ~/Downloads/OpenNI-MacOSX-x64-2.2
sudo ./install.sh
cp -r include /usr/local/include/OpenNI
cp Redist/libOpenNI2.dylib /usr/local/lib


4) Clone the repository for OpenNI2-FreenectDriver:
---------------------------------------------------

git clone https://github.com/piedar/OpenNI2-FreenectDriver ~/Downloads/OpenNI2-FreenectDriver
cd ~/Downloads/OpenNI2-FreenectDriver
./waf configure build


4.5) Copy the driver dylib into OpenNI's drivers directory:
-----------------------------------------------------------

cp build/libFreenectDriver.dylib ../OpenNI-MacOSX-x64-2.2/Redist/OpenNI2/Drivers/
cp -r ../OpenNI-MacOSX-x64-2.2/Redist/OpenNI2 /usr/local/lib


5) Download NiTE 2.2.0.11 body tracking library from:  http://www.openni.org/files/nite/
----------------------------------------------------------------------------------------

6) Install NiTE:
----------------

cd ~/Downloads/NiTE-MacOSX-x64-2.2
sudo ./install.sh
cp ./Redist/libNiTE2.dylib /usr/local/lib
cp -r include /usr/local/include/NiTE/


7) Install Boost:
-----------------

brew install boost
cp -r /usr/local/Cellar/boost/1.55.0/include/boost /usr/local/include
mkdir /usr/local/lib/boost
cp -r /usr/local/Cellar/boost/1.55.0/lib/libboost_system-mt.dylib /usr/local/lib
cp -r /usr/local/Cellar/boost/1.55.0/lib/libboost_thread-mt.dylib /usr/local/lib


8) Install liblo:
-----------------

brew install liblo


9) NiTE Data files:
-------------------

- Build and run the Xcode project
- The Xcode log should say "Could not find data file ./NiTE2/s.dat" and then list its current working directory
- Copy the directory ~/Downloads/NiTE-MacOSX-x64-2.2/Redist/NiTE2 into what Xcode lists as its current working directory









