# CS5220 Project 2: Socket-based ARQ Protocol Programming

[View on Github](https://github.com/ross-nordstrom/cs5220-socket-arq) - (Email me your Github handle if you'd like access)

## One-Liner

If you'd like to build and run a handful of permutations of the client and server, simply run:


### UDP

This dumps useful output files to `./pkg/udp`

```
./configure.sh
./udp.sh
```

### TCP

This dumps useful output files to `./pkg/tcp`. It serves up `README.md` then `PDDModel_Network99.pdf`.

```
./configure.sh
./tcp.sh
```

This runs 16 permutations of frame size, protocol, loss rate, and files. Useful logs are dumped in `./out`, including the retrieved files, and the calculated `diff` of what the server sent from the input file.

Additionally, that directory is packaged up to `out.tar.gz` and dumped into `./pkg`, along with `client.o` and `server.o`. If you have access to the Github repo, these are what I've attached to each Release.

## Building

Standard stuff:

```
./configure.sh
make udp
```

This will generate `client.o` and `server.o` in the top-level directory. Additionally, peripheral binaries will be generated in `./bin`.

## Usage

### UDP

#### Server

```
Usage: ./server.o [-p port] drop_rate protocol
   drop_rate: [0-100], the percent chance of dropping a packet
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat
```

#### Client

Please note my addition of `dst_filename` in the client's prompt. This enables me to have more control of it in the script mentioned above.

```
Usage: ./client.o [-s data_size] host [-p port] src_filename dst_filename protocol
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat

```

### TCP

### Server

```
Usage: ./server.o
```

### Client

Outputs the file to `./out`

```
Usage: ./client.o hostname file-name
```