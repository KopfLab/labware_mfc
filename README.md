## About

Data logger and device controller for MFCs (mass flow controllers). Currently supports Alicat type MFCs.

## Commands

For a list of all available commands, please see https://github.com/KopfLab/labware_commands.

## Wiring

The relevant pins for serial communication on the MiniDin connector are 3 (RX), 5 (TX) and 8 (GND) which correspond to 3 (TX), 2 (RX) and 5 (GND) on the DB-9 side of a serial cable. Most serial cables are not wired correctly so it's easiest to cut the side of the MFC, find which lines correspond to 2, 3 and 5 on the DB-9 connector and then solder 22 AWG solid core wire to those leads to easily connect to the MFCs.
