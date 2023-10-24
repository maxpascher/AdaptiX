# File storage for older resources

For the purpose of loading assets at replay, this folder functions as another base directory.

This enables a cleaning process of the *Content* folder. Simply move the asset files from the *Content* folder to this folder. 

Make sure to keep the folder structure (i.e. if you have a file like `Content/myFolder/myFile.uasset`, move it to `_old/myFolder/myFile.uasset`).

Move assets using the normal file browser (explorer), not the UE4 internal Content Browser as the latter creates reference files, which need to be handled on git.  