import requests


def get_from_api(url, params):
    response = requests.post(url, params=params)

    if response.status_code == 200:
        data = response.json()

        return data["result"]
    else:
        print("request failed!")
        return response.status_code
