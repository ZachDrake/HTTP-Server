# HTTP-Server
Simple HTTP Server - Implemented parts of RFC 2616

Server listens on tcp port 80 and waits for an HTTP request

Server handles GET and HEAD requests appropriately

Upon recieving a GET/HEAD requests the server returns
- 200 OK along with requested file (assuming it exists in www/)
- 404 Not Found with a 404 HTML page

Example Usage:
Compile program with gcc - gcc main.c
Launch program - sudo ./a.out (sudo required because we are listening on port 80)
Visit http://localhost/index.html on your browser

