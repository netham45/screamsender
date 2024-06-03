# ScreamSender

This is a utility originally intended to run on LG WebOS TVs, however it should run anywhere Linux is available. This utility takes in PCM from stdin and sends it out to the specified Scream receiver or [ScreamRouter](http://github.com/netham45/ScreamRouter)

This does not currently support any channel layout other than stereo.

## Usage

* `-i <IP Address>` IP Address to send stream to, Mandatory
* `-p <Port>` Port to send stream to, Mandatory
* `-s <Sample Rate>` Sample Rate, Default 48000
* `-b <Bit Depth>` Bit Depth, Default 32

## Building

### Local build

Tested using Debian 12 and Alma 9 containers

* Install the following packages: `g++` `gcc`
* Compile with `g++ screamsender.cpp -o screamsender`

### Cross-compile for aarch64

Tested using a Debian 12 container

* Install the following packages: `g++-aarch64-linux-gnu` `gcc-aarch64-linux-gnu` `binutils-aarch64-linux-gnu`
* Compile with `aarch64-linux-gnu-g++ --static screamsender.cpp -o screamsender_aarch64`

## To export a stereo audio stream from an LG WebOS TV

Tested on an LG C1

`arecord -B 0 -D "hw:1,2" -f S32_LE -c 2 -r 48000 - | screamsender -i <IP Address> -p <Port>`

Note: Reading from some inputs with arecord with high channel counts set will crash and reboot the TV.

### Capturing audio from HDMI

To capture audio from HDMI you need to have the audio going through the TV -- Playing from either TV, Wired Headphones, or Bluetooth, not ARC/eARC. Optical is untested. Only stereo is available from the TV.

