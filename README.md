# Stratego

## Introduction
This project was developped by Antonin LIONNET, Romain DAVID, and Quentin GAVOILLE, as a project for the Licence 3 Informatique diploma (French equivalent of Bachelor's Degree in Computer Science). This was developped under the supervision of our tutor Julien BERNARD at the University of Franche-Comté in Besançon, France.

The objective of this project was to develop the strategy game Stratego, make users able to play it in a network.

## Dependencies
This project is developped using Gamedev Framework, and will require it to run. You can find GF at https://github.com/GamedevFramework/gf. For documentation on how to install GF, please check this page of GF's documentation: https://gamedevframework.github.io/v0.22.0/build_and_install.html

## How to use Stratego
First, clone the repository using
```
git clone https://github.com/AntoMelon/Stratego.git
```
or download it as a ZIP directly from Github.

Then, if you downloaded a ZIP file, extract it. You can then either:
- use the init.sh script to build the files in a new 'build' directory
- create your own directory to build your files in, using:
```
mkdir yourdir
cd yourdir
cmake yourpath/Stratego
make
```

If building fails, check that you have at least GF v0.21.0 installed, and all GF dependencies.

Once the files are built, you can launch the server using:
```
./stgserver [port]
```
Port is facultative, and should be an available port on your machine. By default, it will be 4269.

To launch a client, use:
```
./stgclient [ip|hostname] [port]
```
The first argument is the IPv4 address or hostname the server is located on. If you launched it from another terminal on your machine, it will be "localhost", corresponding to the address 127.0.0.1. Port should be the port your provided in the ./stgserver command. It is facultative too, and by default is 4269 to match the server default one.
