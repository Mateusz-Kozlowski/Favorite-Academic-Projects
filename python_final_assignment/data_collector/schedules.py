import csv
import json
import requests

from data_collector import common
from data_collector import const


def save_stop_coords(stop, csv_writer):
    # Initialize an empty dictionary to hold the stop data
    stop_info = {}
    # Convert the list of key-value dictionaries to a single dictionary
    for kv in stop["values"]:
        stop_info[kv["key"]] = kv["value"]

    # Extract fields using the correct keys
    stop_id = stop_info.get("zespol")
    stop_number = stop_info.get("slupek")
    stop_name = stop_info.get("nazwa_zespolu")
    latitude = stop_info.get("szer_geo")
    longitude = stop_info.get("dlug_geo")

    # Write the stop data to the CSV file
    csv_writer.writerow([
        stop_id,
        stop_number,
        stop_name,
        latitude,
        longitude
    ])


def save_stops_coords(stops_coords, stops_coords_csv_path):
    path = stops_coords_csv_path

    # Open the file in write mode
    with open(path, 'w', newline='', encoding='utf-8') as file:
        # Create a CSV writer object
        csv_writer = csv.writer(file)

        # Write the header to the CSV file
        csv_writer.writerow([
            'StopID',
            'StopNumber',
            'StopName',
            'Latitude',
            'Longitude'
        ])

        # Loop through each stop and write its data to the CSV
        for stop in stops_coords:
            save_stop_coords(stop, csv_writer)

    print(f"Stops coordinates have been saved to {stops_coords_csv_path}")


def get_stops_coords(stops_coords_csv_path):
    print('Getting stops coords')

    stops_coords = common.get_from_api(
        const.STOPS_COORDS_URL,
        None
    )

    if isinstance(stops_coords, list):
        save_stops_coords(stops_coords, stops_coords_csv_path)
    else:
        print("Failed getting stops coords")


def save_to_json(path, data):
    with open(path, 'w', encoding='utf-8') as json_file:
        json.dump(data, json_file, ensure_ascii=False, indent=4)


def extract_values_from_result(result):
    for values_list in result:
        for value_dict in values_list['values']:
            if 'value' in value_dict:
                yield value_dict['value']


def get_stop_line(busstopId, busstopNr, stops_data):
    # Define the parameters for the API call
    # This is the ID for the method to get lines at a stop
    params = {
        'id': '88cd555f-6f31-43ca-9de4-66c479ad5942',
        'busstopId': busstopId,
        'busstopNr': busstopNr,
        'apikey': const.API_KEY
    }

    # Make the API call to get available lines at the stop
    result = common.get_from_api(
        const.SCHEDULES_ENDPOINT,
        params
    )

    print("id =", busstopId, "nr =", busstopNr, "res:", result)

    # Process the result if it's valid
    if result:
        # Construct a unique key for the stop
        stop_key = f"{busstopId}_{busstopNr}"
        # Extract the lines from the result and add to the
        # stops_data dictionary
        stops_data[stop_key] = list(
            extract_values_from_result(result)
        )


def get_stops_lines(stops_coords_csv_ath, output_json_path, requests_limit):
    print('Getting stops lines:')

    requests_cnt = 0

    stops_data = {}  # Dictionary to hold the data

    with open(stops_coords_csv_ath, mode='r', encoding='utf-8') as csv_file:
        csv_reader = csv.DictReader(csv_file)

        # Iterate over each row in the CSV
        for row in csv_reader:
            requests_cnt += 1
            if requests_cnt > requests_limit:
                break

            busstopId = row['StopID']
            busstopNr = row['StopNumber']

            get_stop_line(busstopId, busstopNr, stops_data)

    save_to_json(output_json_path, stops_data)

    print(f"Stops lines saved to {output_json_path}")


def make_concise_schedule(schedule_dict):
    # A new dictionary to store the concise version
    concise_schedule_dict = {}

    # Loop over the schedule_dict items
    for stop_line, schedule_list in schedule_dict.items():
        # This will hold a list of concise schedules for each stop_line
        concise_schedule_list = []

        # Now, iterate over the list of schedule entries
        for schedule_entry in schedule_list['result']:
            # Create a dictionary for each schedule entry,
            # only include items where the value is not 'null'
            concise_schedule = {
                value_dict['key']: value_dict['value']
                for value_dict in schedule_entry['values']
                if value_dict['value'] != 'null'
            }
            # Add the concise dictionary to the list for this stop_line
            concise_schedule_list.append(concise_schedule)

        # Assign the list of concise schedules to the stop_line key in
        # the main dictionary
        concise_schedule_dict[stop_line] = concise_schedule_list

    return concise_schedule_dict


def get_schedule_for_stop_and_line(stop_id, stop_number, line, schedule_dict):
    # Define the parameters for the API call
    params = {
        'id': 'e923fa0e-d96c-43f9-ae6e-60518c9f3238',
        'busstopId': stop_id,
        'busstopNr': stop_number,
        'line': line,
        'apikey': const.API_KEY
    }

    print("Getting schedule for", stop_id, stop_number, line)

    response = requests.get(const.SCHEDULES_ENDPOINT, params=params)

    if response.status_code == 200:
        # Process the successful response:
        schedule_data = response.json()

        schedule_key = f"{stop_id}_{stop_number}_{line}"
        schedule_dict[schedule_key] = schedule_data
    else:
        error_message = (
            f"Failed to retrieve schedule for Line {line} "
            f"at Stop {stop_id}-{stop_number}"
        )
        print(error_message)


def get_schedules_for_stops_lines(
        stops_lines_json_path,
        schedules_json_path,
        requests_limit):
    print('Getting schedules for stops lines')

    # Load the JSON data into a Python dictionary
    with open(stops_lines_json_path, 'r') as file:
        stops_lines_data = json.load(file)

    schedule_dict = {}
    requests_cnt = 0

    # Iterate through the dictionary
    for stop_key, lines in stops_lines_data.items():
        stop_id, stop_number = stop_key.split('_')

        for line in lines:
            requests_cnt += 1
            if requests_cnt > requests_limit:
                break

            get_schedule_for_stop_and_line(
                stop_id, stop_number,
                line,
                schedule_dict
            )

    save_to_json(schedules_json_path, make_concise_schedule(schedule_dict))
    print("schedules saved to", schedules_json_path)


def get_lines_paths(lines_routes_json_path):
    print('Getting lines paths')

    response = requests.get(
        const.LINES_PATHS_URL,
        params=const.LINES_PATHS_PARAMS
    )

    if response.status_code == 200:
        data = response.json()
        save_to_json(lines_routes_json_path, data)
        print('Lines paths saved to', lines_routes_json_path)
    else:
        print(f"Failed to fetch data, status code: {response.status_code}")


def get_schedules_data(
        stops_coords_csv_path,
        stops_lines_json_path,
        schedules_json_path,
        lines_paths_json_path):
    get_stops_coords(stops_coords_csv_path)
    get_stops_lines(
        stops_coords_csv_path,
        stops_lines_json_path,
        10e9
    )
    get_schedules_for_stops_lines(
        stops_lines_json_path,
        schedules_json_path,
        10e9
    )
    get_lines_paths(lines_paths_json_path)
