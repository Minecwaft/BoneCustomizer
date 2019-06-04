# BoneCustomizer

# Issues
* Does not work on Windows 7 due to Microsoft not supporting it. Work around is to find the copy of XINPUT1_3.dll on your computer and copy it inside the same folder as TERA.exe and rename it to XINPUT1_4.dll
* Does not work with fullscreen mode. The issue here is the way I'm hooking into the directx calls. I'm doing it very generically by creating a device and deleting it quickly after. The problem is that there can be only one device per fullscreen program so it fails and throws an error. 

# How to Use

## Easy way
* Download the .dll from release tab and inject with ANY injector. Done!

## Build it yourself

* Build with the latest MSVC compiler. More easily done by installing Microsoft Visual Studio 2019 and building it in RELEASE mode and 32-BIT. After building simply inject the .dll!

## Optional Toolbox Integration
* Make a script that calls injector.exe which injects the .dll with a custom command

# Notes
* I don't plan on supporting this further although I will fix any offsets that get broken due to updates. Shouldn't happen unless BHS memes.
