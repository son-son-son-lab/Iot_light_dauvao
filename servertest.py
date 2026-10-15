from http.server import BaseHTTPRequestHandler, HTTPServer
import json

HOST = "192.168.1.7"

PORT = 8000

class HTTPtest(BaseHTTPRequestHandler): 
    def do_GET(self):
        self.send_response(200) 
        self.send_header("Content-type", "text/html")
        self.end_headers() 
        self.wfile.write(bytes("<html><body><h1>REAL VO DICH!!!!</h1></body></html>","utf-8"))

server = HTTPServer((HOST,PORT),HTTPtest)
print("server now running...")
server.serve_forever()
server.server_close()
print("server has stopped!")



                               