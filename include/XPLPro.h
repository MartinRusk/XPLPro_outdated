/*
  XPLPro.h - Library for serial interface to Xplane SDK.
  Created by Curiosity Workshop, Michael Gerlicher,  2020-2023

  See readme.txt file for information on updates.

  To report problems, download updates and examples, suggest enhancements or get technical support, please visit:

     discord:  https://discord.gg/gzXetjEST4
     patreon:  www.patreon.com/curiosityworkshop

*/

#ifndef XPLPro_h
#define XPLPro_h

#define XPL_FLOATPRECISION 6 // how many decimals of precision for floating point datarefs.  More increases dataflow (default 2)
#define XPL_RX_TIMEOUT 500         // after detecting a frame header, how long will we wait to receive the rest of the frame.  (default 500 ms)
#define XPL_RESPONSE_TIMEOUT 90000 // after sending a registration request, how long will we wait for the response.
                                   // this is giant because sometimes xplane says the plane is loaded then does other stuff for a while. (default 60000 ms)

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // add to this for boards that need it
#define XPL_USE_PROGMEM                                                                                                     // define this for boards with limited memory that can use PROGMEM to store strings.
                                                                                                                            // You will need to wrap your dataref names with F() macro ie:
                                                                                                                            // Xinterface.registerDataref(F("laminar/B738/annunciator/drive2"), XPL_READ, 100, 0, &drive2);
                                                                                                                            // Disable for boards that have issues compiling: errors with strncmp_PF for instance.
typedef const __FlashStringHelper XPString_t;
#endif

#define XPLMAX_PACKETSIZE 200 // Probably leave this alone.  If you need a few extra bytes of RAM it could be reduced, but it needs to
                              // be as long as the longest dataref name + 10.  If you are using datarefs
                              // that transfer strings it needs to be big enough for those too.  (default 200)

//////////////////////////////////////////////////////////////
// STOP! Dont change any other defines in this header!
//////////////////////////////////////////////////////////////

#ifdef XPL_USE_PROGMEM
// use Flash for strings, requires F() macro for strings in all registration calls
  typedef const __FlashStringHelper XPString_t;
#else
  typedef const char XPString_t;
#endif

#define XPL_BAUDRATE 115200   // don't mess with this, it needs to match the plugin which won't change
#define XPL_PACKETHEADER '['  // ...or this
#define XPL_PACKETTRAILER ']' // ...or this

// Items in caps generally come from xplane.  Items in lower case are generally sent from the arduino.

#define XPLCMD_SENDNAME 'N'            // plugin request name from arduino
#define XPLRESPONSE_NAME 'n'    // Arduino responds with device name as initialized in the "begin" function

#define XPLREQUEST_REGISTERDATAREF 'b' //
#define XPLREQUEST_REGISTERCOMMAND 'm' // just the name of the command to register
#define XPLRESPONSE_DATAREF 'D' // Plugin responds with handle to dataref or - value if not found.  dataref handle, dataref name
#define XPLRESPONSE_COMMAND 'C' // Plugin responds with handle to command or - value if not found.  command handle, command name

#define XPLCMD_PRINTDEBUG 'g'   // plugin logs string sent from arduino
#define XPLCMD_RESET 'z'
#define XPLCMD_SPEAK 's'               // plugin speaks string through xplane speech

#define XPLREQUEST_NOREQUESTS 'c'      // nothing to request
#define XPLREQUEST_UPDATES 'r'         // arduino is asking the plugin to update the specified dataref with rate and divider parameters
#define XPLREQUEST_SCALING 'u'         // arduino requests the plugin apply scaling to the dataref values
#define XPLREQUEST_UPDATESARRAY 't'

#define XPLCMD_DATAREFUPDATEINT '1'
#define XPLCMD_DATAREFUPDATEFLOAT '2'
#define XPLCMD_DATAREFUPDATEINTARRAY '3'
#define XPLCMD_DATAREFUPDATEFLOATARRAY '4'
#define XPLCMD_DATAREFUPDATESTRING '9'

#define XPLCMD_SENDREQUEST 'Q' // plugin sends this when it is ready to register bindings
// #define XPLCMD_DEVICEREADY         'g'
// #define XPLCMD_DEVICENOTREADY      'h'
#define XPLCMD_COMMANDSTART 'i'
#define XPLCMD_COMMANDEND 'j'
#define XPLCMD_COMMANDTRIGGER 'k' //  %3.3i%3.3i   command handle, number of triggers

#define XPL_EXITING 'X' // MG 03/14/2023: xplane sends this to the arduino device during normal shutdown of xplane.  It may not happen if xplane crashes.

#define XPL_DATATYPE_INT 1
#define XPL_DATATYPE_FLOAT 2
#define XPL_DATATYPE_STRING 3

#define XPL_HANDLE_INVALID -1

class XPLPro
{
public:
  XPLPro(Stream *);
  void begin(const char *devicename, void (*initFunction)(void), void (*stopFunction)(void), void (*inboundHandler)(int)); // parameter is name of your device for reference

  int connectionStatus(void);
  int commandTrigger(int commandHandle);                   // triggers specified command 1 time;
  int commandTrigger(int commandHandle, int triggerCount); // triggers specified command triggerCount times.
  int commandStart(int commandHandle);                     // Avoid this unless you know what you are doing.  Command "begins" must be balanced with command "ends"
  int commandEnd(int commandHandle);

  void datarefWrite(int handle, int value);
  void datarefWrite(int handle, int value, int arrayElement);

  void datarefWrite(int handle, long int value);
  void datarefWrite(int handle, long int value, int arrayElement);

  void datarefWrite(int handle, float value);
  void datarefWrite(int handle, float value, int arrayElement);

  void requestUpdates(int handle, int rate, float precision);
  void requestUpdates(int handle, int rate, float precision, int element);

  void setScaling(int handle, int inLow, int inHigh, int outLow, int outHigh);

  int registerDataRef(XPString_t *datarefName);
  int registerCommand(XPString_t *commandName); // register a command

  int sendDebugMessage(const char *);
  int sendSpeakMessage(const char *msg);

  void sendResetRequest(void);
  int xloop(void); // where the magic happens!

  long datarefReadFloat() { return _readValueFloat; }
  long datarefReadInt() { return _readValueLong; }
  long datarefReadElement() { return _readValueElement; }

private:
  void _processSerial();
  void _processPacket();

  void _transmitPacket();

  void _sendname();

  void _sendPacketVoid(int command, int handle);        // just a command with a handle
  void _sendPacketString(int command, const char *str); // send a string

  int _parseInt(int *outTarget, char *inBuffer, int parameter);
  int _parseInt(long int *outTarget, char *inBuffer, int parameter);
  int _parseFloat(float *outTarget, char *inBuffer, int parameter);
  int _parseString(char *outBuffer, char *inBuffer, int parameter, int maxSize);

  Stream *streamPtr;

  char _receiveBuffer[XPLMAX_PACKETSIZE];
  int _receiveBufferBytesReceived;
  char _sendBuffer[XPLMAX_PACKETSIZE];

  byte _registerFlag;
  byte _connectionStatus;

  void (*_xplInitFunction)(void);  // this function will be called when the plugin is ready to receive binding requests
  void (*_xplStopFunction)(void);  // this function will be called with the plugin receives message or detects xplane flight model inactive
  void (*_xplInboundHandler)(int); // this function will be called when the plugin sends dataref values

  const char *_deviceName;

  int _handleAssignment;

  // int readValueInt;
  long _readValueLong;
  float _readValueFloat;
  int _readValueElement;
};

extern XPLPro XP;

#endif
