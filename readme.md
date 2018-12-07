## Asepsis == solution for .DS_Store pollution.

This is my personal fork of [Asepsis](http://www.github.com/binaryage/asepsis), which adds support for recent macOS versions (up to 10.14 Mojave), and cleans up the project so it's a bit easier to work with.

All credit should go to BinaryAge for the original development of Asepsis, they've done all the hard work. I have only done some small changes to make it more comfortable for me to use. I'm putting it out here to maybe make someone's life easier.

#### For end-user info please visit [http://asepsis.binaryage.com](http://asepsis.binaryage.com).

#### License: [MIT-Style](license.txt)

### Use at your own risk!
Asepsis replaces parts of the system frameworks, and even though it does multiple backups during installation, and appears to work for me and other users, this is a potentially dangerous modification (which should be obvious from the need to disable System Integrity Protection), so be careful.

Tested and working on High Sierra and Mojave.

### Installation from sources

#### Requirements
- Xcode 10
- SIP disabled ([info and guide](https://totalfinder.binaryage.com/sip))

To build and install, download the repository and open its directory in Terminal, and then run:

    rake build
    rake install

You need to reboot your computer for Asepsis to come into effect.

### Uninstallation

From Terminal, run:

    asepsisctl uninstall

You need to reboot your computer for the uninstallation to finish.
