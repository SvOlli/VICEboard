TODO
====

Sooner:
- migrate GPIO tester to main program
  - actually use EEPROM gpio configuration
  - manual edit for gpio configuration
- clean up menus

Later:
- figure out how to get battery level
  - set bluetooth property
- set bluetooth property as "Keyboard"
- get charging mode
  - handle LEDs during charging when in deep sleep
- more diagnostic output
  - ESP.getChipRevision()
  - ESP.getSdkVersion()
  - ESP.getSketchSize()
  - ESP.getSketchMD5()
  - ESP.getFreeSketchSpace()
  - ESP.magicFlashChipSize()
- check for wireless USB keyboard option
- when the reset button is exported as an additional key (at the side),
  a full power off option would be useful
  - configuration option to decide between deep sleep and power off
