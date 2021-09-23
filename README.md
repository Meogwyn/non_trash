non_trash

This is an Arduino serial debugger/console with a couple of features the serial interface that 
comes with the Arduino IDE doesn't have

The main feature of this interface is the ability to split the input into several parts by writing sequentially to a user-defined 
number of boxes on the right terminal. Thus if we receive three-byte transmissions from the Arduino, we can configure the right
terminal to write into the so-called "div boxes" sequentially until it reaches the final box and jumps back to the first box.

You might want to use this feature if you have a transmission with a particular number of bytes.

Possible future features include:
1. Detecting changes and stopping, similarly to how an oscilloscope would react to a trigger in single seq. mode
2. Writing output to log files (in compressed form)
3. As it stands you can send particular byte strings to the arduino from the left terminal. A possible new feature would be
the option to include 'protocols' which will be user-provided functions to send messages to the Arduino. Perhaps the 
'protocols' could be in the format of shared object files...
