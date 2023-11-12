#!/usr/bin/python3
# -*- coding: utf-8 -*-
import requests

def print_request(request):
    req = "{method} {path_url} HTTP/1.1\r\n{headers}\r\n{body}".format(
        method = request.method,
        path_url = request.path_url,
        headers = ''.join('{0}: {1}\r\n'.format(k, v) for k, v in request.headers.items()),
        body = request.body or "",
    )
    return "{req_size}\n{req}\r\n".format(req_size = len(req), req = req)

#POST multipart form data
def post_multipart(host, port, namespace, headers, payload):
    req = requests.Request(
        'POST',
        'https://{host}:{port}{namespace}'.format(
            host = host,
            port = port,
            namespace = namespace,
        ),
        headers = headers,
        data = payload,
    )
    prepared = req.prepare()
    return print_request(prepared)

if __name__ == "__main__":
    #usage sample below
    #target's hostname and port
    #this will be resolved to IP for TCP connection
    host = 'localhost'
    port = '8080'
    namespace = '/api/v1/prog/change_percent'
    #below you should specify or able to operate with
    #virtual server name on your target
    headers = {
        'Accept': 'application/json',
        'Content-type': 'application/json'
    }
    payload = {
        'percent':16
    }

    print(post_multipart(host, port, namespace, headers, payload))
