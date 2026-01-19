# HTTP Server
A simple HTTP server written from scratch in C.

## Dependencies
The only requirements for compilation are `gcc` and `make`.

### Linux (Ubuntu)
```
sudo apt install build-essential
```

## Building and Running
To build the application, run the following commands:
```bash
git clone https://github.com/justint9696/http-server.git
cd http-server
make

# Run tests (optional)
./tests.sh
```

After a successful compilation, the runtime application will be located in `bin`. Running `bin/server --help` will provide the necessary parameters.

## Directory Overview
If you're interested in browsing the code, the directory structure is as follows:
1. `http-server` - the actual server code
2. `public` - the web server modules
3. `src` - the runtime application for the server
4. `test` - all the unit tests for the server code
