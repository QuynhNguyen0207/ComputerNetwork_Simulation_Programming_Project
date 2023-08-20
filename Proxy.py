# Include the libraries for socket and system calls
import socket
import sys
import os
import argparse
import re


# Library to extract and calculate age
import email.utils as eut
import datetime
from pytz import timezone

# helper function 1
# return current time based on the timeZone given
def getCurrentTime(timeZone):
  return datetime.datetime.now(timeZone).replace(tzinfo=None)

# helper function 2
# parse date to have the same format as the date returned from getCurrentTime()
def parseDate(date):
  return datetime.datetime(*eut.parsedate(date)[:6])

# 1MB buffer size
BUFFER_SIZE = 1000000

parser = argparse.ArgumentParser()
parser.add_argument('hostname', help='the IP Address Of Proxy Server')
parser.add_argument('port', help='the port number of the proxy server')
args = parser.parse_args()

# Create a server socket, bind it to a port and start listening
# The server IP is in args.hostname and the port is in args.port
# bind() accepts an integer only
# You can use int(string) to convert a string to an integer
# ~~~~ INSERT CODE ~~~~
# call HOST variable as a name for args.hostname
HOST = args.hostname 
# convert a string to an integer for args.port & call PORT variable as a name for integer of args.port
PORT = int(args.port) 
# ~~~~ END CODE INSERT ~~~~

try:
  # Create a server socket
  # ~~~~ INSERT CODE ~~~~
  # Define s is the socket to connect
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
  # ~~~~ END CODE INSERT ~~~~
  print('Connected socket')
except:
  print('Failed to create socket')
  sys.exit()

try:
  # Bind the server socket to a host and port
  # ~~~~ INSERT CODE ~~~~
  # bind the server socket to HOST and PORT as the name of the args.hostname and integer of the args.port
  s.bind((HOST,PORT)) 
  # ~~~~ END CODE INSERT ~~~~
  print('Port is bound')
except:
  print('Port is in use')
  sys.exit()

try:
  # Listen on the server socket
  # ~~~~ INSERT CODE ~~~~
  # Listening on the server socket one at a time
  s.listen(1) 
  # ~~~~ END CODE INSERT ~~~~
  print('Listening to socket')
except:
  print('Failed to listen')
  sys.exit()

while True:
  print('\n\nWaiting connection...')

  clientSocket = None
  try:
    # Accept connection from client and store in the clientSocket
    # ~~~~ INSERT CODE ~~~~
    # The connection from client is accepted from the server and stored in the clientSocket
    clientSocket, addr = s.accept() 
    # ~~~~ END CODE INSERT ~~~~
    print('Received a connection from:', args.hostname)
  except:
    print('Failed to accept connection')
    sys.exit()

  clientRequest = 'METHOD URI VERSION'
  # Get request from client
  # and store it in clientRequest
  # ~~~~ INSERT CODE ~~~~
  # Client send request which could be stored in clientRequest with 1024, the maximum bytes of data that can be stored in clientRequest
  request = clientSocket.recv(1024) 
  clientRequest = request.decode()
  # ~~~~ END CODE INSERT ~~~~

  print('Received request:')
  print('< ' + clientRequest)

  # Extract the parts of the HTTP request line from the given message
  requestParts = clientRequest.split()
  method = requestParts[0]
  URI = requestParts[1]
  version = requestParts[2]
  
  print('Method:\t\t' + method)
  print('URI:\t\t' + URI)
  print('Version:\t' + version)
  print('')
  print('')

  # Remove http protocol from the URI
  URI = re.sub('^(/?)http(s?)://', '', URI, 1)

  # Remove parent directory changes - security
  URI = URI.replace('/..', '')

  # Split hostname from resource
  resourceParts = URI.split('/', 1)

  hostname = resourceParts[0]
  resource = '/'

  if len(resourceParts) == 2:
    # Resource is absolute URI with hostname and resource
    resource = resource + resourceParts[1]

  print('Requested Resource:\t' + resource)

  cachePath = './' + hostname + resource
  if cachePath.endswith('/'):
    cachePath = cachePath + 'default'

  print('Cache location:\t\t' + cachePath)

  fileExists = os.path.isfile(cachePath)

  try:
    # Check wether the file exist in the cache
    # read the cache
    cacheFile = open(cachePath, "r")
    cacheData = cacheFile.readlines()

    print('Cache hit! Loading from cache file: ' + cachePath)
    
    # ProxyServer finds a cache hit
    # handle cache directives in the header field
    # Check if the cache is suitable to re-use (Any "cache-control" header in the cache?)
    # If the cache can be re-use, send back contents of cached file
    # ~~~~ INSERT CODE ~~~~
    # find data in cacheData and send this data to clientSocket when requested
    for data in cacheData: 
      clientSocket.send(data)
    # ~~~~ END CODE INSERT ~~~~
    
    cacheFile.close()
    print('cache file closed')

  # Error handling for file not found in cache and cache is not suitable to send
  except IOError:
    originServerSocket = None
    # Create a socket to connect to origin server
    # and store in originServerSocket
    # ~~~~ INSERT CODE ~~~~
    # create origin server socket to connect to origin server if the file is not found in cache
    originServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    # ~~~~ END CODE INSERT ~~~~

    print('Connecting to:\t\t' + hostname + '\n')
    try:
      # Get the IP address for a hostname (origin server)
      hostAddress = socket.gethostbyname(hostname)

      # Connect to the origin server
      # ~~~~ INSERT CODE ~~~~
      # Connect to the origin server socket with the hostAddress and port
      originServerSocket.connect((hostAddress, 80)) 
      # ~~~~ END CODE INSERT ~~~~

      print('Connected to origin Server')

      # Create a file object associated with this socket
      # This lets us use file function calls
      originServerFileObj = originServerSocket.makefile('+', 0)

      # Create origin server request line and headers to send
      # and store in originServerRequestHeader and originServerRequestLine
      originServerRequestLine = ''
      originServerRequestHeader = ''
     
      # originServerRequestLine is the first line in the request and
      # originServerRequestHeader is the second line in the request
      # ~~~~ INSERT CODE ~~~~
      # Create a origin server request line with the instruction from the extract the parts of the HTTP request line from the given message
      originServerRequestLine = method + ' ' + resource + ' ' + version 
      # Create a origin server request header with the instruction from the printing part of creating a socket to connect to origin server
      originServerRequestHeader = 'Host: ' + hostname 
      # ~~~~ END CODE INSERT ~~~~

      # Construct the request to send to the origin server
      originServerRequest = originServerRequestLine + '\r\n' + originServerRequestHeader + '\r\n\r\n'

      # Request the web resource from origin server
      print('Forwarding request to origin server:')
      for line in originServerRequest.split('\r\n'):
        print('> ' + line)

      try:
        originServerSocket.sendall(originServerRequest)
      except socket.error:
        print('Send failed')
        sys.exit()

      print('Request sent to origin server\n')
      originServerFileObj.write(originServerRequest)

      # use to store response from the origin server
      data  = ''
      
      # Get the response from the origin server
      # ~~~~ INSERT CODE ~~~~
      # using data to get the response from the origin server by using the recv method with 2000 which is the avaiable maximum bytes of data to receive at one 
      data = originServerSocket.recv(2000) 
      # ~~~~ END CODE INSERT ~~~~

      # use to determine if this response should be cached?
      isCache = True

      # Get the response code from the response
      dataLines = data.split('\r\n')
      responseCode = dataLines[0] 

      # Decide which content should be cached
      # ~~~~ INSERT CODE ~~~~
      # if responseCode == 200, 301, 302, the file should be cached, otherwise it should not be cached
      print('responseCode: ', responseCode)
      if '200' in responseCode:
        isCache = True
      elif '301' in responseCode:
        isCache = True
      elif '302' in responseCode:
        isCache = True
      else:
        isCache = False
      # ~~~~ END CODE INSERT ~~~~
      
      # Send the data to the client
      # ~~~~ INSERT CODE ~~~~
      # Using send method to send the data to the client
      clientSocket.send(data) 
      # ~~~~ END CODE INSERT ~~~~

      # cache the content if it should be cached
      if isCache:
        # Create a new file in the cache for the requested file.
        # Also send the response in the buffer to client socket
        # and the corresponding file in the cache
        cacheDir, file = os.path.split(cachePath)
        print('cached directory ' + cacheDir)
        if not os.path.exists(cacheDir):
          os.makedirs(cacheDir)
        cacheFile = open(cachePath, 'wb')

        # Save origin server response (data) in the cache file
        # ~~~~ INSERT CODE ~~~~
        # using for loop through data which should contain the response received from the origin server
        for i in data: 
          # using the write method to write every element of the data to the cacheFile
          cacheFile.write(i) 
        # ~~~~ END CODE INSERT ~~~~

        cacheFile.close()
        print('cache file closed')

      # finished sending to origin server - shutdown socket writes
      originServerSocket.shutdown(socket.SHUT_WR)
      
      print('origin server done sending')
      originServerSocket.close()
      
      clientSocket.shutdown(socket.SHUT_WR)
      print('client socket shutdown for writing')

    except IOError, (value, message): 
      print('origin server request failed. ' + message)
  try:
    clientSocket.close()
  except:
    print('Failed to close client socket')