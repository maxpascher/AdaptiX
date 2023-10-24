# Adaptive Folder

This folder stores everything external related to the Adaptive Elements of control.

This includes:

- [ml-remote-server](./ml-remote-server): A fork of [git@github.com:getnamo/ml-remote-server.git](git@github.com:getnamo/ml-remote-server.git), adapted to serve our needs
- [model](./model): The model used for adaptive control
- [StartupServer.bat](./StartupServer.bat): The batch file to run the server.

For this to run, be sure to have python 3 installed and the requirements of the adaptive models met.

## Adaptive Usage:

0. Run the server using the [StartupServer.bat](./StartupServer.bat) file. This can be active for multiple simulation runs.
1. Start the Simulation. 
   If visualisation is wanted, make sure to toggle the Adaptive Axes Visualisation to be active. For this, click on the VR_Pawn and click on the Button in the Adaptive category or use the Hotkey (ButtonMapping: **STRG + H**)
2. Connect to Jaco (ButtonMapping: **X**)
3. Connect to Adaptive Server (ButtonMapping: **U**)
   This will also activate the cameras
4. Update Adaptive Axes using either 
   - Single Update (ButtonMapping: **Z**), or
   - Automatic regular update (ButtonMapping: **T**, abort with **G**)

