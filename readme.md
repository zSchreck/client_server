This is code for an http web server and an http client. 

**TO RUN THE CLIENT:**
The client can be run by going within the 'client/' directory and typing make. 
This will compile the code into an executable titled 'client', and this can be run by typing './client [ARGS]' where '[ARGS]' are the specified arguments that can be passed in.

Any data returned by the http request will be written to a file titled 'response' which is located within the client directory. To remove the client executable or the response file, simply type 'make clean'.

**IMPORTANT NOTE ON CLIENT:** url passed in MUST begin with http://.



**TO RUN THE SERVER:**
The server can be run by going into the 'server/' directory and typing make. 
This will create an executable file titled 'server' which can then be run by typing in './server'.

**IMPORTANT NOTES ON SERVER:**
The server will be running at 127.0.0.1 at port 8000. To send requests to the server from the client, first launch the server, then pass in http://127.0.0.1:8000/[filename] where '[filename]' is the name of the file you wish to retrieve with your http request. The file must be located within the server directory for the requests to work correctly. 
