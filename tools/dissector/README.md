# Wireshark DeadMaze Dissector

A Wireshark dissector for DeadMaze protocol

## Build

Using the top-level CMake with default configuration should compile this dissector

## Installation

This dissector need to be placed in [Wireshark Plugin folder](https://www.wireshark.org/docs/wsug_html_chunked/ChPluginFolders.html)

`make install` will install it in the system folder, if you want to install it only for your user, on Linux,
the path is **${HOME}/.local/wireshark/plugins/<version>/epan**

## Usage

The dissector will outomatically dissect DeadMaze protocole. If you want to force decode,
use `Decode As -> DeadMaze`.
