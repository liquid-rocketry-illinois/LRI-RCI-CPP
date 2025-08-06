# Rocket Control Interface (RCI) Usage Guide

RCI is a graphical utility for interacting with target devices using RCP. It provides an interface for collecting and
graphing output from sensors, controlling and viewing the state of actuators, and starting and managing test
procedures.

When in an active configuration, multiple smaller windows will display details about the configured peripherals (see
configuration docs [here](./json.md)), which can be moved and combined to the user's preference. Each window is
composed of subwindows, which contain one type of information. Each subwindow type is documented below.

## RCI vs RCP

RCI is a standalone executable program that displays the graphical interface and handles communications with a
target device. This is what end users will interact with. RCP stands for Rocket Control Protocol, and it is the
underlying grammar that defines how a target and host device communicate. End users do not need to be concerned with
RCP, as this is a process internal to RCI and cannot be directly interacted with. Both the target and host device
(RCI) will implement RCP, which allows them to communicate.

## Timed buttons

In order to prevent misclicks from sending unintentional packets, some buttons must be held down to active. When 
hovered, these buttons will show a tooltip saying "Hold to confirm". Near the button, a small circle will fill as 
the button is held, and when it completes, the action will be performed. Note that if you continue to hold the 
button for long enough, the button will re-trigger.

# Subwindows

## Target Selector subwindow

When RCI is first started, it needs to know what configuration to load, and how it should communicate with the
target device. This subwindow is not combinable with other subwindows, and is always on screen, as it is how you
control the overall functionality of RCI.

The first line in the target selector subwindow shows the current connection status. This will either say `CLOSED`
or `OPEN`, indicating if a connection to a target device has been opened or not. If no connection is open, the
target selector will be the only UI element on screen.

### Closed state

In the closed state, the next line lets you choose which target configuration to load. Each configuration defines the
available peripherals and the window layout for a given physical target. For example, `test_stand.json` is the
configuration for the RAND-E test stand, and defines all the devices on the stand, as well as how the subwindows should
be laid out and organized.

The next line selects what type of communication interface to use. There are several options available:

#### Serial Port

Connect to the target over a USB (or RS232) serial port. The next line will show the available serial devices you
can connect to, and this list can be refreshed with the refresh button below it. The baud rate can be set by
adjusting the baud rate field. When all options are set, click connect.

_Be careful what device you connect to!_ On some systems, the bluetooth card or other devices can appear as a serial
device in the list, so make sure you check what youre connecting to beforehand!

#### TCP Socket

Connect to the target over a TCP/IP socket. This connection method can run in either server or client mode, i.e.
whether RCI is the TCP server and should start listening on the specified port for incoming connections, or RCI is
the client and should connect to a TCP server at the given IP address/port. This information is all filled in to the
fields in the target selector window. The mode can be changed by the 2 buttons.

If in server mode, make sure you either know your computer's current IP address, or have set a known static IP.

#### Virtual Port

This is a "connection" option for debugging and demonstration purposes only. It will imitate an RCP compliant device
by sending the ready packet, but otherwise it will simply sink all RCP packets sent by RCI.

### Open State

In the open state, this window will display the currently selected configuration and interface.

While in the open state, you can adjust the number of RCP packets per frame RCI processes. If you are connecting to a
target which sends a lot of data back, such as a sensor board, the sensor graphs may lag behind if the polling rate is
not set high enough, as packets are coming in at a faster rate than RCI is processing them. However, increasing the
polling rate will increase the time between rendered frames, resulting in visual lag. The time to render an 
individual frame is displayed as `Latest Frame Time`, and should be at most 0.02 seconds.

The close interface button at the bottom will close the communication interface with the target, and reset RCI to 
the startup state.

## EStop Window

This window comprises only of an EStop button that takes up all available content room. When pushed, it sends an 
EStop command to the target device.

## Test Control Window

This window allows you to control the active auto test, heartbeats, and whether sensor data is streamed back to the 
host. The `Select Test` dropdown allows you to select which test to start, and the control buttons (`Start`, `End`, 
`Pause`, and `EStop`) allow you to control the running state of the test. 

To enable sensor data streaming, check the `Enable Data Streaming` box.

To enable heartbeats, check the `Send Heartbeat Packets` box, set the time between heartbeats, and click confirm.

If the `Reset Sensor Time Base on Start` box is checked, then the clock of the target is reset to now, so sensor 
data will begin to be logged at time 0 again.

## Debug Info and Prompts Window

This window shows the debug output from the target, i.e. any textual logging sent by the target. The output window 
can be cleared by pressing the `Clear` button.

Below that, any prompts started from the target will be shown. Boolean values and decimal numbers can be inputted to 
the target from here.

## Sensor Information Window

This window shows data from received sensors. Each sensor is contained within a dropdown. The title of the dropdown 
shows the name of the sensor. 

When the dropdown is open, the sensor status will be displayed, which indicates if any data has been received from 
the sensor or not.

All data received can be written to a CSV file with `Write to CSV`.

The number of received data points is shown next to that.

The value of the latest received datapoint is shown below that.

The next line displays the different components of the sensor data that can be tared. For example, a sensor that 
graphs X, Y, and Z data will have separate tare buttons for each channel of data.

Below that the all received data can be cleared with the clear button.

In each graph, the view size and other options can be changed in the right-click context menu.

## Actuator control windows

The two actuator control window types (simple actuators (on or off), stepper control, angled actuator control 
(servos)) are all formatted very similarly. Each will display the name of the device, and a status indicating if the 
data is stale, or if the actuator is active. An actuators state can be changed with the nearby buttons.
