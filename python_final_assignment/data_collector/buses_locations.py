import time
import csv

from data_collector import common
from data_collector import const


def append_buses_data_to_file(
        list_id: int,
        buses_data: list[dict[str, str]],
        file_path: str):
    print(list_id)

    with open(file_path, 'a', newline='', encoding='utf-8') as file:
        writer = csv.writer(file, delimiter=';')

        for bus_data_dict in buses_data:
            # we add reading id as the 1st column
            # and then the entire bus data dictionary values
            row = [list_id] + list(bus_data_dict.values())

            writer.writerow(row)


def try_to_append_request_res(buses_data, buses_data_path, elapsed_time):
    if isinstance(buses_data, list):
        print("appending a new list")

        append_buses_data_to_file(
            elapsed_time // const.BUSES_UPDATE_PERIOD,
            buses_data,
            buses_data_path
        )
    else:
        print("got NOT a list:", buses_data)


def requests_loop(duration, buses_data_path):
    elapsed_time = 0

    while elapsed_time < duration:
        print("getting buses data from api")

        try_to_append_request_res(
            common.get_from_api(
                const.BUSES_LOC_URL,
                const.BUSES_LOC_REQUEST_PARAMS
            ),
            buses_data_path,
            elapsed_time
        )

        if duration < const.BUSES_UPDATE_PERIOD:
            return

        time.sleep(const.BUSES_UPDATE_PERIOD)
        elapsed_time += const.BUSES_UPDATE_PERIOD


def get_buses_loc_data(duration, buses_data_path):
    headerList = [
        'ReadingID',
        'Lines',
        'Lon',
        'VehicleNumber',
        'Time',
        'Lat',
        'Brigade'
    ]

    with open(buses_data_path, 'w', newline='', encoding='utf-8') as file:
        dw = csv.DictWriter(file, delimiter=';', fieldnames=headerList)
        dw.writeheader()

    requests_loop(duration, buses_data_path)
