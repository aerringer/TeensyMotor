#!/opt/local/bin/python
import re 
from serial.tools import list_ports

import serial
import sys
import threading
import wx
import json
import traceback
import time


SERIALRX = wx.NewEventType()
# bind to serial data receive events
EVT_SERIALRX = wx.PyEventBinder(SERIALRX, 0)

class SerialRxEvent(wx.PyCommandEvent):
    eventType = SERIALRX
    def __init__(self, windowID, data):
        wx.PyCommandEvent.__init__(self, self.eventType, windowID)
        self.data = data

    def Clone(self):
        self.__class__(self.GetId(), self.data)

#----------------------------------------------------------------------
FORWARD = 0
REVERSE = 1

class ControlParameters(object):
    def __init__(self):
        self.rate = 0
        self.maxRate = 0
        self.rampRange = 0
        self.targetRate = 0
        self.maxPwmValue = 0
        self.motorPwm = 0
        self.direction = FORWARD
        self.maxPosition = 0
        self.desiredPosition = 0
        self.minPosition = 0
        self.resetZeroPoint = 0
        self.stop = 0
        self.start = 0
        self.scheme = ""

    def get_as_json(self):
        return json.dumps(self.__dict__, separators=(',',':'))

class MotorFrame(wx.Frame):
    def __init__(self, *args, **kwds):
        self.serialPort = None
        self.thread = None
        self.alive = threading.Event() 
        self.controlParameters = ControlParameters()
        self.currentControlMessage = ""
        self.lastControlMessage = ""

        kwds["style"] = wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)
        
        self.rampRangeLabel = wx.StaticText(self, label="RampRange", style=wx.ALIGN_CENTRE)
        self.rampRange = wx.TextCtrl(self, -1, "20")
        self.targetRateLabel = wx.StaticText(self, label="TargetRate", style=wx.ALIGN_CENTRE)
        self.targetRate = wx.TextCtrl(self, -1, "20")
        self.rateLabel = wx.StaticText(self, label="Rate", style=wx.ALIGN_CENTRE)
        self.rate = wx.TextCtrl(self, -1, "20")
        self.rate.SetEditable(False)
        self.maxRateLabel = wx.StaticText(self, label="MaxRate", style=wx.ALIGN_CENTRE)
        self.maxRate = wx.TextCtrl(self, -1, "10")
        self.maxPwmValueLabel = wx.StaticText(self, label="MaxPwmValue", style=wx.ALIGN_CENTRE)
        self.maxPwmValue = wx.TextCtrl(self, -1, "30")
        self.motorPwmLabel = wx.StaticText(self, label="MotorPwm", style=wx.ALIGN_CENTRE)
        self.motorPwm = wx.TextCtrl(self, -1, "10")
        self.motorPwm.SetEditable(False)
        self.directionLabel = wx.StaticText(self, label="Direction", style=wx.ALIGN_CENTRE)
        self.direction = wx.CheckBox(self, -1)
        self.maxRotationLabel = wx.StaticText(self, label="MaxRotation", style=wx.ALIGN_CENTRE)
        self.maxRotation = wx.TextCtrl(self, -1, "1000")
        self.minRotationLabel = wx.StaticText(self, label="MinRotation", style=wx.ALIGN_CENTRE)
        self.minRotation = wx.TextCtrl(self, -1, "0")

        self.button_ok = wx.Button(self, -1, "OK")
        self.Bind(wx.EVT_BUTTON, self.OnClickOk, self.button_ok)
        self.button_ok.SetDefault()
        self.button_ok.SetSize(self.button_ok.GetBestSize())
        
        self.button_stop = wx.Button(self, -1, "Stop")
        self.Bind(wx.EVT_BUTTON, self.OnClickStop, self.button_stop)
        self.button_stop.SetSize(self.button_stop.GetBestSize())
        
        self.button_start = wx.Button(self, -1, "Start")
        self.Bind(wx.EVT_BUTTON, self.OnClickStart, self.button_start)
        self.button_start.SetSize(self.button_start.GetBestSize())
        
        self.button_reset = wx.Button(self, -1, "Reset")
        self.Bind(wx.EVT_BUTTON, self.OnClickReset, self.button_reset)
        self.button_reset.SetSize(self.button_reset.GetBestSize())

        self.text_ctrl_output = wx.TextCtrl(self, -1, "", style=wx.TE_MULTILINE|wx.TE_READONLY)
        self.out_slider = wx.Slider(self, 
                           value=0, \
                           minValue=-100, \
                           maxValue=10100, \
                           style=wx.SL_HORIZONTAL| wx.SL_AUTOTICKS)

        self.in_slider = wx.Slider(self, 
                           value=0, \
                           minValue=-100, \
                           maxValue=10100, \
                           style=wx.SL_HORIZONTAL| wx.SL_AUTOTICKS)

        self.out_slider.SetTickFreq(5, 1)
        self.out_slider.Bind(wx.EVT_SCROLL, self.OnOutSliderScroll)
        self.in_slider.SetTickFreq(5, 1)
        self.in_slider.Bind(wx.EVT_SCROLL, self.OnInSliderScroll)

        self.in_slider_value = wx.StaticText(self, label="0")
        self.out_slider_value = wx.StaticText(self, label="0")

        sampleList = ['Bouncer', 'Absolute']

        self.scheme = wx.Choice(self, -1, (100, 50), choices = sampleList)
        self.Bind(wx.EVT_CHOICE, self.OnEvtScheme, self.scheme)


        self.Show()
        self.Bind(EVT_SERIALRX, self.OnSerialRead)
        self.StartThread()
        self.__set_properties()
        self.__do_layout()


    def __set_properties(self):
        # begin wxGlade: TerminalFrame.__set_properties
        self.SetTitle("Teensy Motor Control")
        self.SetSize((900, 500))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: TerminalFrame.__do_layout
        sizer_1 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_1.Add(self.rampRangeLabel, 1, wx.EXPAND, 0)
        sizer_1.Add(self.rampRange, 1, wx.EXPAND, 0)
        sizer_1.Add(self.targetRateLabel, 1, wx.EXPAND, 0)
        sizer_1.Add(self.targetRate, 1, wx.EXPAND, 0)
        sizer_1.Add(self.rateLabel, 1, wx.EXPAND, 0)
        sizer_1.Add(self.rate, 1, wx.EXPAND, 0)
        sizer_1.Add(self.maxRateLabel, 1, wx.EXPAND, 0)
        sizer_1.Add(self.maxRate, 1, wx.EXPAND, 0)
        sizer_1.Add(self.motorPwmLabel, 1, wx.EXPAND, 0)
        sizer_1.Add(self.motorPwm, 1, wx.EXPAND, 0)

        sizer_2 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_2.Add(self.maxPwmValueLabel, 1, wx.EXPAND, 0)
        sizer_2.Add(self.maxPwmValue, 1, wx.EXPAND, 0)
        sizer_2.Add(self.maxRotationLabel, 1, wx.EXPAND, 0)
        sizer_2.Add(self.maxRotation, 1, wx.EXPAND, 0)
        sizer_2.Add(self.minRotationLabel, 1, wx.EXPAND, 0)
        sizer_2.Add(self.minRotation, 1, wx.EXPAND, 0)
        sizer_2.Add(self.directionLabel, 1, wx.EXPAND, 0)
        sizer_2.Add(self.direction, 1, wx.EXPAND, 0)

        sizer_3 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_3.Add(self.scheme, 1, 0, 0)
        sizer_3.Add((1,1), 1, 0, 0)
        sizer_3.Add(self.button_ok, 1, 0, 0)
        sizer_3.Add(self.button_start, 1, 0, 0)
        sizer_3.Add(self.button_stop, 1, 0, 0)
        sizer_3.Add(self.button_reset, 1, 0, 0)
        sizer_3.Add((1,1), 1, 0, 0)

        sizer_4 = wx.BoxSizer(wx.VERTICAL)
        sizer_4.Add(sizer_1, 0, wx.EXPAND, 0)
        sizer_4.Add(sizer_2, 0, wx.EXPAND, 0)
        sizer_4.Add((1,1), 0, wx.EXPAND, 0)
        sizer_4.Add(sizer_3, 0, wx.EXPAND, 0)
        sizer_4.Add(self.out_slider_value, 0, wx.EXPAND, 0)
        sizer_4.Add(self.out_slider, 0, wx.EXPAND, 0)
        sizer_4.Add(self.in_slider_value, 0, wx.EXPAND, 0)
        sizer_4.Add(self.in_slider, 0, wx.EXPAND, 0)
        sizer_4.Add(self.text_ctrl_output, 8, wx.EXPAND, 0)

        self.SetAutoLayout(1)
        self.SetSizer(sizer_4)
        self.Layout()
        # end wxGlade

    def OnClickStart(self, eventId):
        pass

    def OnClickStop(self, eventId):
        pass

    def OnClickReset(self, eventId):
        pass

    def OnEvtScheme(self, eventId):
        pass

    def OnClickOk(self, eventId):
        self.controlParameters.rampRange = int(self.rampRange.GetValue())
        self.controlParameters.targetRate = int(self.targetRate.GetValue())
        self.controlParameters.maxRate = int(self.maxRate.GetValue())
        self.controlParameters.maxPwmValue = int(self.maxPwmValue.GetValue())
        self.controlParameters.direction = int(self.direction.GetValue())
        self.controlParameters.maxPosition = int(self.maxRotation.GetValue())
        self.controlParameters.minPosition = int(self.minRotation.GetValue())
        self.controlParameters.scheme = self.scheme.GetStringSelection()
        self.controlParameters.desiredPosition = self.in_slider.GetValue()
        self.currentControlMessage = self.controlParameters.get_as_json()
        self.in_slider.SetRange(self.controlParameters.minPosition, self.controlParameters.maxPosition)
        self.out_slider.SetRange(self.controlParameters.minPosition, self.controlParameters.maxPosition)

    def OnInSliderScroll(self, eventId):
        objectId = eventId.GetEventObject()
        val = objectId.GetValue()
        
        self.in_slider_value.SetLabel(str(val))

    def OnOutSliderScroll(self, eventId):
        objectId = eventId.GetEventObject()
        val = objectId.GetValue()
        
        self.out_slider_value.SetLabel(str(val))

    def find_port(self):
        a = list(list_ports.comports())
        for portStruct in a:
            m = re.search('(/dev/cu\.usbmodem.*)', portStruct[0])
            if m:
                print "found port: ", m.group(0)
                return m.group(0)

    def OnSerialRead(self, event):
        text = event.data
        print "rec'd %s" %(text)
        self.text_ctrl_output.AppendText(text)
        self.text_ctrl_output.AppendText("\n")
        try:
            m = json.loads(text)
        except ValueError as e:
            print traceback.format_exc()
            print "carrying on"
            return

        if m.has_key("C"):
            self.out_slider.SetValue(m["C"])
            self.out_slider_value.SetLabel(str(m["C"]))
        if m.has_key("E"):
            self.motorPwm.SetValue(str(m["E"]))
        if m.has_key("D"):
            self.direction.SetValue(True if m["D"] == 0 else False)
        if m.has_key("R"):
            self.rate.SetValue(str(m["R"]))
        if m.has_key("Error"):
            print "got error: %s" % m["Error"]
        if m.has_key("Info"):
            print "got info: %s" % m["Info"] 
     
    def StartThread(self):
        """Start the receiver thread"""        
        self.thread = threading.Thread(target=self.ComPortThread)
        self.thread.setDaemon(1)
        self.alive.set()
        self.thread.start()

    def StopThread(self):
        """Stop the receiver thread, wait util it's finished."""
        if self.thread is not None and self.thread.is_alive():
            self.alive.clear()          #clear alive event for thread
            try:
                self.thread.join()          #wait until thread has finished
            except RuntimeError as e:
                print "Join failed."
            self.thread = None

    def ComPortThread(self):
        """Thread that handles the incomming traffic. Does the basic input
           transformation (newlines) and generates an SerialRxEvent"""
        while self.alive.isSet():   
            if not self.serialPort:
                try:
                    self.serialPort = serial.Serial(self.find_port(), timeout=1)
                except Exception as e:
                    print traceback.format_exc()
                    print "Exception - Waiting..."
                    time.sleep(1)
                    continue
                except OSError as e:
                    print "OSError - Waiting..."
                    time.sleep(1)
                    continue

            try:            #loop while alive event is true
                text = ""
                chars = self.serialPort.read(1)          #read one, with timeout
                if chars:                            #check if not timeout
                    n = self.serialPort.inWaiting()     #look if there is more to read
                    chars += self.serialPort.read(n)
                    for char in chars:
                        if ord(char) == 13:
                            pass
                        elif ord(char) == 10:
                            if len(text) == 0:
                                print "Got an empty line"
                                continue
                            event = SerialRxEvent(self.GetId(), text)
                            self.GetEventHandler().AddPendingEvent(event)
                            text = ""

                            if self.currentControlMessage  != self.lastControlMessage:
                                self.lastControlMessage = self.currentControlMessage
                                print "Sending to Teensy", self.currentControlMessage
                                self.serialPort.write(self.currentControlMessage)
                                self.serialPort.write("\n")
                        else:
                            text = text + char

            except serial.SerialException as e:
                print "waiting..."
                self.serialPort = None
                time.sleep(1)

            except Exception as e:
                print traceback.format_exc()
                self.StopThread()
                exit(0)


class TeensyMotorApp(wx.App):
    def OnInit(self):
        frame_terminal = MotorFrame(None)
        self.SetTopWindow(frame_terminal)
        frame_terminal.Show(1)
        return 1

if __name__ == '__main__':
    app = TeensyMotorApp(0)
    app.MainLoop()  