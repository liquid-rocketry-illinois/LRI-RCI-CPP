# Target Configuration JSON Schema

This file describes the schema of the target config files. The format must be followed exactly, otherwise a config file
is invalid, and program behavior is indeterminate (likely a crash).

## Table 1: Main Fields

| Field Name |       Type       | Description                                                                                                                                                                       |
|:-----------|:----------------:|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| name       |      String      | The name of the configuration. Used only for display purposes                                                                                                                     |
| tests      | Array of Objects | The set of valid tests and their associated RCP ID numbers from [0, 15]. See table 2 for object details                                                                           |
| devices    | Array of Objects | Used to configure the singletons with what devices are present and should be loaded. Two objects in this array cannot share the same device class. See table 3 for object details |
| windows    | Array of Objects | Used to configure the windowlets and which modules they contain. See table 4 for details                                                                                          |

## Table 2: Tests object array fields

| Field Name |     Type     | Description                     |
|:-----------|:------------:|:--------------------------------|
| id         | int, [0, 15] | The RCP ID of the test          |
| name       |    String    | Human readable name of the test |

## Table 3: Devices object array fields

| Field Name |       Type       | Description                                                                                                                                                                              |
|:-----------|:----------------:|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| devclass   |       int        | The RCP device class associated with the device. Classes 0 and 0x80 do not need to be specified for them to function                                                                     |
| ids        |   Array of Int   | Indicates the IDs to load. Even if there is only one of a particular device class this still needs to be an array of ints                                                                |
| names      | Array of Strings | Matches human readably names to the IDs of the devices. Must be in the same order as the IDs array and must be the same size. At least one name is required to match the one required ID |

The Boolean Sensor device (0x95) has one additional field, `refreshTime`. This field is an integer that sets the 
interval, in seconds to refresh sensors. If set to zero, use the default.

## Table 4: Windowlet object array fields

| Field Name |       Type       | Description                                                                                                                                                                                                    |
|:-----------|:----------------:|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| title      |      String      | Title of the windowlet                                                                                                                                                                                         |
| modules    | Array of Objects | Gives an ordered list of what modules should appear in that windowlet. Each object has a type field which indicates the module type, the rest of the fields depend on this type field. See table 5 for details |

## Table 5: Windowlet Modules object array fields

| Type Number |       Module Name        | Other Fields |
|:-----------:|:------------------------:|:-------------|
|     -1      |          ESTOP           | None         |
|      0      |       Test Control       | None         |
|      1      | Simple Actuator Controls | See Table 6  |
|      2      |  Stepper Motor Controls  | See Table 6  |
|      3      |      Prompt Display      | None         |
|      4      | Angled Actuator Display  | See Table 6  |
|  128/0x80   |     Data Log display     | None         |
|    0x95     |  Boolean Sensor Display  | See Table 6  |
| 0x90 - 0xC0 |   Sensor Value Viewer    | See Table 7  |

## Table 6: Simple Actuator, Stepper Motor Control, Boolean Sensor Fields

| Field Name  |     Type      | Description                                                                                                      |
|:------------|:-------------:|:-----------------------------------------------------------------------------------------------------------------|
| refresh     |     bool      | Whether or not to display a refresh button at the top                                                            |
| ids         | Array of ints | Which RCP ID devices to display. IDs specified in this array must be enumerated in the devices structure as well |

The Angled Actuator module does not contain the refresh field.

## Table 7: Sensor Value Field Viewer

| Field Name |       Type       | Description                                                                                                            |
|:-----------|:----------------:|:-----------------------------------------------------------------------------------------------------------------------|
| abridged   |       bool       | Whether this sensor module should be rendered in abridged mode                                                         |
| ids        | Array of Objects | Which sensors to track. Each object contains the information for a range of IDs per device class to track. See Table 8 |

## Table 8: Sensor Module "ids" Object Fields

| Field Name |     Type      | Description                                                 |
|:-----------|:-------------:|:------------------------------------------------------------|
| devclass   |      int      | The device class of this object and the corresponding IDs   |
| ids        | Array of Ints | The IDs of the above device class of the individual sensors |
   