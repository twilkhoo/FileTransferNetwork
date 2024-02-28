# File Transfer Network

## Description
This is a file transfer network emulating FTP, built in C++.
The server supports GET and PUT requests for clients.
The negotiation/control port is decided through UDP, and the actual file transfer is done through TCP.

## Main Usage
To compile the files and run the executables, run:
```
make
./server.sh <storage_directory>
./client.sh <server_address> <n_port> <command> <filename>
```

- `<storage_directory>` is a local directory on the server's machine where files may be stored. In the demo, we use `./serverfiles`.
- `<server_address>` is the name of the machine that is running the server (`localhost`, if server and client are running on the same machine, or something like ubuntu2204-012.student.cs.uwaterloo.ca)
- `<n_port>` is the negotiation port that the server opened, which it outputs to stdout as `SERVER_PORT=XXXXX`.
- `<command>` is one of {GET, PUT}
- `<filename>` is the name of the file to get from the server, or to put to the server.

Note: before this, you may have to `chmod +x server.sh && chmod +x client.sh`.

## Other Usage
Some other ways to use this library, notably the client-side commands, include:
```
./client.sh <server_address> <n_port> <command> <filename> <receive_directory>
```
This defines the directory for the client to store and receive the files from the server in. By default, this is a local directory called `./clientfiles`.

```
./client.sh <server_address> <n_port> KILL
```
This gracefully terminates the server.

## Demo
https://github.com/twilkhoo/FileTransferNetwork/assets/30396273/072f37a1-4a3b-4947-8b2e-90970fd1b2c1

## Next Steps
- Multithread the server to handle requests concurrently.
- Add more request types beyond GET and PUT.
