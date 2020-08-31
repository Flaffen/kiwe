kiwe -- a multi-threaded webserver written in C. This webserver is an extension of [Lambda School's webserver practice project](https://github.com/LambdaSchool/C-Web-Server), which is not multi-threaded and uses a single TCP connection per single HTTP request.
# Installation
Clone the repository and run ``make`` in the server folder. **Note**: default C compiler for the project is Clang, but you can change it in the Makefile.

# Planned Features
 - [WSGI](https://en.wikipedia.org/wiki/Web_Server_Gateway_Interface) support
 - Reverse proxy capabilities
