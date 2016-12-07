# ESP8266 simple TCP server

This project will connect to a WIFI AP and create a simple TCP server that
will respond to really simple commands from a client.

Here's an attempt at a rough spec:

1.  For each step, do a quick debug print via UART.
2.  Connect to my router.
3.  Once connected, blink the LED on GPIO2.
4.  Create a TCP server that listens on port 8266.
5.  When a client connects, print: `Hello, I'm your ESP8266.\n`
6.  Wait for a command line from the client, which is expected to be
    any newline-terminated string.
7.  Known commands are defined to be everything up to a literal space
    or newline character, whichever comes first. Anything after a space,
    up until a newline character, is the argument for that command.
    Supported commands will be:
    *   `hello` - Ignore the argument and respond with: `Hi there.\n`
    *   `echo` - Repeat back exactly what the argument is.
    *   `echoup` - Same as `echo` but make the response all uppercase.
    *   `low` - GPIO2 will go low.
    *   `high` - GPIO2 will go high.
    *   `share` - Like `echo` but will send the argument to all clients.
    *   `bye` - Say `Goodbye\n` and drop connection.

