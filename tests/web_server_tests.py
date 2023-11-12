import os
import time
import pytest
from pathlib import Path

import docker

from contextlib import contextmanager

import server_api as utils

client = docker.from_env()

def get_image_name():
    return os.environ['IMAGE_NAME']

# def network_name():
    # return os.environ.get('DOCKER_NETWORK')

@contextmanager
def run_server():
    server_domain = os.environ.get('SERVER_DOMAIN', '127.0.0.1')
    server_port = os.environ.get('SERVER_PORT', '8080')
    docker_network = os.environ.get('DOCKER_NETWORK')

    entrypoint = [
        "/app/web_server",
        "--result-path", "/app/result/",
        "--config-file", "/app/templates/config.json",
        "--script-path", "/app/script/script.py",
        "--certs-path", "/app/certs/"
    ]
    kwargs = {
        'detach': True,
        'entrypoint': entrypoint,
        'auto_remove': True,
        'ports': {f"{server_port}/tcp": server_port},
    }
    if docker_network:
        kwargs['network'] = docker_network
    if server_domain != '127.0.0.1':
        kwargs['name'] = server_domain
    container = client.containers.run(
        get_image_name(),
        **kwargs
    )

    for i in range(2000):
        log = container.logs().decode('utf-8')
        if log.find('server started') != -1:
            break
        time.sleep(0.001)

    server = utils.Server(f'https://{server_domain}:{server_port}/')
    try:
        yield server, container
    finally:
        try:
            container.stop()
        except docker.errors.APIError:
            pass

@pytest.mark.parametrize('method', ['POST'])
def test_simple(method):
    with run_server() as (server, container):
        url = '/api/v1/prog/tag_values'
        body = {
            "contractType": "RUSTONN_PHYS_PERSON",
            "currencyType": "ROUBLES",
            "currencyKind": "CASH",
            "contractDuration": 2
        }
        res: requests.Response = server.post(url, data = body)
        assert res.status_code == 200

@pytest.mark.parametrize('method', ['POST'])
def test_bad_request(method):
    with run_server() as (server, container):
        url = '/app/v1/prog/tag_values'
        body = {}
        res: requests.Response = server.post(url, data = body)
        assert res.status_code == 400

@pytest.mark.parametrize('method', ['POST'])
def test_not_found(method):
    with run_server() as (server, container):
        url = '/srv/private_info'
        res: requests.Response = server.get(url)
        assert res.status_code == 404

# @TODO test_permission_denied and test_internal_server_error

@pytest.mark.parametrize('method', ['POST'])
def test_permission_denied(method): # @TODO
    with run_server() as (server, container):
        url = '/root'
        res: requests.Response = server.get(url)
        assert res.status_code == 403

@pytest.mark.parametrize('method', ['POST'])
def test_internal_server_error(method): # @TODO How can we test it?
    with run_server() as (server, container):
        url = '/'
        res: requests.Response = server.get(url)
        assert res.status_code == 500
