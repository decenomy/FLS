Flits-Core
=============

Setup
---------------------
[Flits-Core](http://FLS.org/wallet) is the original FLS client and it builds the backbone of the network. However, it downloads and stores the entire history of FLS transactions; depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more. Thankfully you only have to do this once.

Running
---------------------
The following are some helpful notes on how to run Flits-Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/flits-qt` (GUI) or
- `bin/flitsd` (headless)

### Windows

Unpack the files into a directory, and then run flits-qt.exe.

### macOS

Drag flits-qt to your applications folder, and then run flits-qt.

### Need Help?

* See the documentation at the [FLS Wiki](https://github.com/Simple-Software-Solutions/Flits-Core/wiki)
for help and more information.
* Ask for help on [BitcoinTalk](https://bitcointalk.org/index.php?topic=1262920.0) or on the [FLS Forum](http://forum.FLS.org/).
* Join our Discord server [Discord Server](https://discord.FLS.org)

Building
---------------------
The following are developer notes on how to build Flits-Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The FLS repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://www.fuzzbawls.pw/FLS/doxygen/)
- [Translation Process](translation_process.md)
- [Unit Tests](unit-tests.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Dnsseed Policy](dnsseed-policy.md)

### Resources
* Discuss on the [BitcoinTalk](https://bitcointalk.org/index.php?topic=1262920.0) or the [FLS](http://forum.FLS.org/) forum.
* Join the [FLS Discord](https://discord.FLS.org).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
