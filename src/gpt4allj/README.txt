This is a modified gpt4all-chat that acts like an simplistic UDP server.
Instead of running a GUI application, it takes queries and send replies
using UDP packets. Therefore you are free to build your client/GUI however you want.
This sever was tested only on Linux. Please do not consider this code ready
for production. It was quickly hacked together just to test the chat client
on PumpkinOS (which was also quickly hacked together :)

The original gpt4all-chat project is here:
https://github.com/nomic-ai/gpt4all-chat

Compile with make and run with:
./chat <model> <port_number>

where <model> is the pathname of the model to be used and <number> is the
UDP port number the server will listen to (it binds to address 0.0.0.0).
An UDP client must send packets containing bare ASCII text to this address/port
and the server will send the replies to the address/port the client has just used.
A single query packet may cause the server to send multiple reply packets.
A packet with string "EOF" (including the null byte ate the end) indicates the
server is done replying to the query.

The server is not multi-threaded and can not handle more than one client at a time.
A new query packet will be processed only after the current query is processed
and all the replies are sent.

Just like in the original gpt4all-chat, this code runs on the CPU, not GPU.
You need a fairly recent and fast CPU to run it, and at least 8GB of RAM
(although more is recommended).

Models are not included in this distribution because of
1) tricky licensing
2) they are huge! (usually 4GB in size).
PLease check the gpt4all site for intructions on how/where to get a model.

In PumpkinOS, the Command application provides an external command "chat",
which is an example of an UDP client that talks to this server.
Check the CommandChat application for more information on how to use it.

I have observed severe performance degradation after a long, continuous exchange of queries
and responses. The same thing happens in the GUI gpt4all application, so the problem lies
within gpt4all implementation, not in the chat command. Sometimes, the responses
become repetitive and appear to enter a never ending loop. In this case, just kill
the chat server and start it again. It will start a fresh, empty conversation context.
The client does not need to be restarted, since it is stateless.

The original readme for gpt4all-chat is README.md.
