# HoloTek

**DISCLAIMER: The code contained in this repository is not optimized nor really pretty. I'm mostly playing with the DLib library and HoloLens.**

## Introduction

HoloLens application that allows a professor from EPITECH to mark student as 'present' at an activity, using Facial Recognition.  
  
This was a project I developed for fun to explore the development on HoloLens using only C++ WinRT and DirectX.  
This project uses the C++ Library [dlib](http://dlib.net/) for the facial detection and recognition of the students.  
Dlib is under the [Boost Software License](http://dlib.net/license.html).

## Installation

To use this projects, you have to install the [dlib](http://dlib.net/) in this folders:
- `vendor/lib` -> the `dlib.lib` file
- `vendor/include` -> `dlib` folder containing all the headers of the library

## Repo hierarchy

I developed this project in multiple steps:  
- **DesktopIntra** is a UWP desktop application interacting with the REST api of the intranet of my school. It allowed me to develop utility functions and test them easily using a Xaml UI.
- **DesktopTek** is a UWP desktop application. I experimented on the [DLib]([dlib](http://dlib.net/)) Facial Detection and Recognition features, and linked all that with the REST api of the intranet
- **HoloTek** is the UWP HoloLens application. It's the **DesktopTek** application, but using DirectX to render menus
- **Serializer** was used to pre-calculate the referenced images to reduce the time of recognition.

## Face Recognition

Most of the interesting code is inside the `HoloTek/Recognition/FacesBuffer.cpp` file.  
  
It works pretty well on the Desktop application, but the processing speed of the HoloLens is too slow: it takes approximately 15 seconds to process a frame.  
  
## Axes of improvement
- Send the camera stream to a remote server that will do the processing and recognition of images

## Contact

I'm not planning in pursuing this project for now.  
If you have questions do not hesitate to contact me either on my Twitter account (@oxydros) or via e-mail: oxydros at gmail.