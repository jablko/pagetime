#include "application.h"

// Things to remember between iterations of loop() below
boolean connecting = false;
boolean responding = false;

// The magic words to send to iamresponding.com.  These might get
// filled in in one iteration of loop() but used in another iteration
char *data;

// Thing for speaking across the Internets
TCPClient client;

// iamresponding.com IP address
byte server[] = { 209, 15, 211, 90 };

// Called once at startup
void setup()
{
	// There are internal pull-up/-down resistors?  Handy!
	pinMode(D0, INPUT_PULLDOWN);
}

// Called in a loop forever
void loop()
{
	// When you push the button
	if (digitalRead(D0) == HIGH)
	{
		// It might take a couple of loop() iterations to
		// successfully connect to iamresponding.com
		connecting = true;

		// This is how to control the Spark's RGB LED
		RGB.control(true);

		// Red to indicate we're attempting to connect
		RGB.color(255, 0, 0);

		// Push the button once to respond to station.
		// Push it again to cancel.
		if (responding)
		{
			// Do just what the iamresponding.com app does
			data = "{\"keyEntryDesc\":\"CANCELLED\",\"memberIDS\":\"\",\"subscriberID\":\"\",\"userfullname\":\"Jack Bates\"}";
		}
		else
		{
			data = "{\"keyEntryDesc\":\"Station\",\"memberIDS\":\"\",\"subscriberID\":\"\",\"userfullname\":\"Jack Bates\"}";
		}
	}

	// We haven't successfully connected since the button was last
	// pushed, keep trying!
	if (connecting && client.connect(server, 80)) // HTTP is port
	{					      // number 80, FYI
		// We successfully connected, stop trying
		connecting = false;

		// Yellow to indicate we successfully connected and
		// sent the magic words
		RGB.color(255, 255, 0);

		// Again, just ape the iamresponding.com app
		client.println("POST /v3/keyEntries.asmx/AddCallEntry HTTP/1.0");

		client.print("Content-Length: ");
		client.println(strlen(data));

		client.println("Content-Type: application/json");
		client.println("Host: iamresponding.com");
		client.println();

		// Send the magic words
		client.print(data);
	}

	// iamresponding.com sent something back to *us*.
	// (This is our only indication that TCPClient is done
	// transmitting the magic words.  You're welcome to get clever
	// and check if iamresponding.com sent back confirmation or is
	// reporting an error, this just assumes it's confirmation.)
	if (client.available())
	{
		// If we were already responding, then now we aren't
		// (cancelled) and vice versa (if we weren't, then now
		// we are).
		responding = !responding;
		if (responding)
		{
			// Blue to indicate that iamresponding.com
			// knows we're responding to station!
			RGB.color(0, 0, 255);
		}
		else
		{
			// Restore the default RGB LED behavior when
			// we're not responding (cancelled).
			// (By default it shows the Wi-Fi status.)
			RGB.control(false);
		}

		// Tricky: Clean up after transmitting the magic
		// words.  Close the connection to iamresponding.com
		// and discard (flush) anything sent back to us, or
		// else in the next loop() iteration we'll think
		// iamresponding.com sent back something *again*.
		client.stop();
		client.flush();
	}
}
