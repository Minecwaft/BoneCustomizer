# BoneCustomizer

# Issues
* Does not work on Windows 7 due to Microsoft not supporting it. Work around is to find the copy of XINPUT1_3.dll on your computer and copy it inside the same folder as TERA.exe and rename it to XINPUT1_4.dll
* Does not work with fullscreen mode. The issue here is the way I'm hooking into the directx calls. I'm doing it very generically by creating a device and deleting it quickly after. The problem is that there can be only one device per fullscreen program so it fails and throws an error. 
