# TLE to PDB Converter for PocketSat on palmOS
This web tool updates your [PocketSat+](https://palmdb.net/app/pocketsat) app with the latest satellite orbits. It converts live space data from [Celestrak](https://celestrak.org/NORAD/elements/index.php?FORMAT=tle) website directly in your browser so you can load it onto your Palm.

## How to use it ?
 - Pick a satellite list from the menu (100 Brightest, weather, communications, navigation satellites) 
 - Click Generate PDB file.  
 - Transfer the downloaded file to your Palm Pilot via HotSync or an SD card

## Notes: 

The CelesTrak API may block repeated requests in a short time frame. If this happens, an error message will appear directly in the application.

This project is a JavaScript port of a command-line script whose C source code is available in this repository.
