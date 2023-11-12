import requests

from urllib.parse import urljoin

class Server:
    def __init__(self, url: str):
        self.url = url

    def request(self, method, header, url, **kwargs):
        try:
            url = urljoin(self.url, url)
            print(url)
            req = requests.Request(method, url, headers = header, **kwargs).prepare()
            with requests.Session() as session:
                return session.send(req)
        except Exception as ex:
            print(ex)

    def get(self, endpoint):
        return requests.get(urljoin(self.url, endpoint))

    def post(self, endpoint, data):
        return requests.post(urljoin(self.url, endpoint), json = data, verify = False)
