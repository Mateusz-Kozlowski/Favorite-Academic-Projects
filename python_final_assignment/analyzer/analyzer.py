import pandas as pd
import math
import numpy as np
import json

from haversine import haversine, Unit

from analyzer import const
from analyzer import visualizer


def coordinates_valid(latitude, longtitude):
    return -90 <= latitude <= 90 and -180 <= longtitude <= 180


def get_dist(prev_latitude, prev_longtitude, latitude, longtitude):
    prev_coordinates_valid = coordinates_valid(
        prev_latitude,
        prev_longtitude
    )
    current_coordinates_valid = coordinates_valid(
        latitude,
        longtitude
    )

    if prev_coordinates_valid and current_coordinates_valid:
        prev_coords = (prev_latitude, prev_longtitude)
        current_coords = (latitude, longtitude)

        return haversine(prev_coords, current_coords, unit=Unit.KILOMETERS)
    else:
        return 0


def calculate_speed(
        prev_latitude,
        prev_longtitude,
        latitude,
        longtitude,
        delta_time_in_seconds,
        safe_div: bool):
    distance = get_dist(prev_latitude, prev_longtitude, latitude, longtitude)

    delta_time_in_hours = delta_time_in_seconds / 3600

    if safe_div and math.isclose(delta_time_in_hours, 0.0):
        return None
    else:
        return distance / delta_time_in_hours


# the function filters out obvious outliers:
def is_valid_coordinate(lat, lon):
    is_lat_in_bounds = const.INITIAL_MIN_LAT <= lat <= const.INITIAL_MAX_LAT
    is_lon_in_bounds = const.INITIAL_MIN_LON <= lon <= const.INITIAL_MAX_LON

    return is_lat_in_bounds and is_lon_in_bounds


def get_grid_sizes_in_deg(cols_cnt, rows_cnt):
    grid_size_lat = (const.INITIAL_MAX_LAT - const.INITIAL_MIN_LAT)
    grid_size_lat /= rows_cnt

    grid_size_lon = (const.INITIAL_MAX_LON - const.INITIAL_MIN_LON)
    grid_size_lon /= cols_cnt

    return grid_size_lat, grid_size_lon


def latlon_to_grid(lat, lon, grid_size_lat, grid_size_lon):
    lat_grid = (lat - const.INITIAL_MIN_LAT) // grid_size_lat
    lon_grid = (lon - const.INITIAL_MIN_LON) // grid_size_lon

    return lat_grid, lon_grid


def get_total_n_speeding_counts(df, speed_threshold):
    # Group by grid coordinates and count the total number of buses for
    # each group
    total_counts = df.groupby(['grid_lat', 'grid_lon']).size()

    # Group by grid coordinates and count the number of speeding buses
    # for each group
    speeding_counts = df[df['Speed'] > speed_threshold].groupby(
        ['grid_lat', 'grid_lon']
    ).size()

    return total_counts, speeding_counts


def get_grid_summary(speeding_percentage, total_counts, speeding_counts):
    # Merge the total counts with the speeding percentages for complete
    # information
    grid_summary = speeding_percentage.merge(
        total_counts.reset_index(name='total_count'),
        on=['grid_lat', 'grid_lon']
    )

    # Now grid_summary contains the total counts and the percentage of
    # speeding incidents for each grid square
    # Add a column for speeding counts to the grid summary
    grid_summary = grid_summary.merge(
        speeding_counts.reset_index(name='speeding_count'),
        on=['grid_lat', 'grid_lon'],
        how='left'
    )

    # Fill NaN values with 0 for grid squares where there are no
    # speeding counts
    grid_summary['speeding_count'] = grid_summary['speeding_count'].fillna(0)

    # Convert the speeding_count to an integer, since counts should be
    # whole numbers
    grid_summary['speeding_count'] = grid_summary['speeding_count'].astype(int)

    # Now grid_summary contains the total counts, the percentage, and
    # the actual count of speeding incidents for each grid square
    print("Grid summary with speeding counts:")
    print(grid_summary)

    return grid_summary


def call_fast_places_visualization(
        df,
        speed_threshold,
        speed_dist_png_path,
        speeding_png_path,
        cols_cnt, rows_cnt):
    total_counts, speeding_counts = get_total_n_speeding_counts(
        df,
        speed_threshold
    )

    # Calculate the raw percentage values, fill NA/NaN values with 0
    raw_percentages = speeding_counts / total_counts * 100
    percentages_with_no_nans = raw_percentages.fillna(0)

    # Reset index and rename the percentage column
    speeding_percentage = percentages_with_no_nans.reset_index(
        name='speeding_percentage'
    )

    visualizer.visualize_speed(
        df,
        speed_dist_png_path,
        get_grid_summary(speeding_percentage, total_counts, speeding_counts),
        speeding_png_path,
        cols_cnt,
        rows_cnt
    )


# Filter the data and ensure the dataframe is not a view or a copy
def adjust_df(df):
    df = df[
        df.apply(
            lambda row: is_valid_coordinate(row['Lat'], row['Lon']), axis=1
        )
    ]
    df = df.copy()
    return df


def places_with_many_fast_buses(
        df,
        speed_dist_png_path,
        speed_threshold,
        speeding_png_path,
        cols_cnt=3, rows_cnt=3):
    df = adjust_df(df)

    grid_size_lat, grid_size_lon = get_grid_sizes_in_deg(cols_cnt, rows_cnt)

    # Calculate grid coordinates for each bus reading and assign them
    # directly to new DataFrame columns
    df[['grid_lat', 'grid_lon']] = df.apply(
        lambda row: latlon_to_grid(
            row['Lat'], row['Lon'],
            grid_size_lat, grid_size_lon
        ),
        axis=1, result_type='expand'
    )

    call_fast_places_visualization(
        df,
        speed_threshold,
        speed_dist_png_path,
        speeding_png_path,
        cols_cnt, rows_cnt
    )


def get_analysis_beginnings(df):
    buffer = pd.Timedelta(minutes=10)

    analysis_beginnings = df.groupby(
        ['VehicleNumber', 'Line', 'Brigade']
    )['Time'].min() + buffer

    return analysis_beginnings


def load_schedules_df(schedules_json_path):
    with open(schedules_json_path, 'r') as file:
        schedules_data = json.load(file)

    schedules_list = []
    for stop_line, schedules in schedules_data.items():
        stop_id, stop_number, line = stop_line.split('_')
        for schedule in schedules:
            schedules_list.append({
                'StopID': stop_id,
                'StopNumber': stop_number,
                'Line': line,
                'Brigade': schedule['brygada'],
                'Direction': schedule['kierunek'],
                'Route': schedule['trasa'],
                'Time': schedule['czas']
            })

    return pd.DataFrame(schedules_list)


def init_next_stops_of_buses(schedules_json_path, analysis_beginnings):
    schedules_df = load_schedules_df(schedules_json_path)

    # Adding a column with time in a format that allows sorting and
    # comparison
    schedules_df['Time'] = pd.to_datetime(
        schedules_df['Time'], format='%H:%M:%S'
    ).dt.time

    # Sorting the DataFrame by
    # 'VehicleNumber', 'Line', 'Brigade', 'Time' columns
    sorted_schedules_df = schedules_df.sort_values(
        by=['VehicleNumber', 'Line', 'Brigade', 'Time']
    )

    # Resetting the index after sorting to facilitate further
    # operations
    sorted_schedules_df.reset_index(drop=True, inplace=True)

    # Now, for each unique triplet of
    # 'VehicleNumber', 'Line', 'Brigade',
    # we find the first stop based on time
    first_stops_df = sorted_schedules_df.drop_duplicates(
        subset=['VehicleNumber', 'Line', 'Brigade']
    )

    # `first_stops_df` now contains rows corresponding to the first
    # stops for each triplet
    return first_stops_df, sorted_schedules_df


def get_lines_routes(lines_routes_json_path):
    with open(lines_routes_json_path, 'r') as file:
        lines_routes_data = json.load(file)

    # Initialize a dictionary to hold the processed routes data
    lines_routes = {}

    # Loop through each line in the JSON data
    for line_id, dict_of_routes in lines_routes_data['result'].items():
        # Initialize a dictionary for each line to hold route mappings
        lines_routes[line_id] = {}
        # Loop through each direction in the line
        for _, route_stops_dict in dict_of_routes.items():
            # Loop through the ordered stops to map each stop to the
            # next
            for i in range(len(route_stops_dict)-1):
                cur_stop = (
                    route_stops_dict[i][1]['nr_zespolu'],
                    route_stops_dict[i][1]['nr_przystanku']
                )

                # Map the current stop to the next stop
                lines_routes[line_id][cur_stop] = (
                    route_stops_dict[i+1][1]['nr_zespolu'],
                    route_stops_dict[i+1][1]['nr_przystanku']
                )

    return lines_routes


def convert_next_stops_to_dict(next_stops_of_buses):
    next_stops_dict = {}

    for _, row in next_stops_of_buses.iterrows():
        key = (row['VehicleNumber'], row['Line'], row['Brigade'])
        value = (row['StopID'], row['StopNumber'], row['Time'])

        next_stops_dict[key] = value

    return next_stops_dict


def bus_near_stop(cur_lon, cur_lat, next_stop_info):
    pass


def calc_lateness_data_frame(
        df,
        next_stops_of_buses,
        sorted_schedules_df,
        lines_routes):
    # Sort df by 'Time' to ensure chronological order
    df = df.sort_values(by='Time').reset_index(drop=True)

    # Convert data frame to dict:
    next_stops_of_buses_dict = convert_next_stops_to_dict(next_stops_of_buses)

    # Przygotowanie DataFrame do zapisywania wyników
    lateness_results = []

    for _, row in df.iterrows():
        vehicle_number = row['VehicleNumber']
        line = row['Line']
        brigade = row['Brigade']
        cur_time = row['Time']
        cur_lat = row['Lat']
        cur_lon = row['Lon']

        vehicle_key = (vehicle_number, line, brigade)

        next_stop_info = next_stops_of_buses_dict[vehicle_key]

        if not next_stop_info:
            continue  # No info about the next stop

        if bus_near_stop(cur_lon, cur_lat, next_stop_info):
            stop_zespol = next_stop_info[0]
            stop_number = next_stop_info[1]
            planned_arrival = next_stop_info[2]

            lateness_results.append({
                'VehicleNumber': vehicle_number,
                'Line': line,
                'Brigade': brigade,
                'StopID': stop_zespol,
                'StopNumber': stop_number,
                'PlannedArrival': planned_arrival,
                'ActualArrival': cur_time
            })

            new_next_stop = lines_routes[line][(stop_zespol, stop_number)]

            next_stops_of_buses_dict[vehicle_key] = (
                new_next_stop[0],
                new_next_stop[1],
                # get_new_next_time()
            )

    return pd.DataFrame(lateness_results)


def get_stops_coords():
    pass


def on_time_analysis(
        df,
        stops_coords_csv_path,
        lines_routes_json_path,
        schedules_json_path):
    print('Time for punctuality analysis:')

    analysis_beginnings = get_analysis_beginnings(df)

    next_stops_of_buses, sorted_schedules_df = init_next_stops_of_buses(
        schedules_json_path,
        analysis_beginnings
    )

    lines_routes = get_lines_routes(lines_routes_json_path)
    stops_coords = get_stops_coords(stops_coords_csv_path)

    calc_lateness_data_frame(
        df,
        next_stops_of_buses,
        sorted_schedules_df,
        lines_routes,
        stops_coords
    )

    print('On time analysis done!')


def get_percentage_within_radius(df, central_point_coords, radius_km):
    # Define a function to calculate whether a point is within the
    # radius:
    def is_within_radius(row, central_point_coords, radius_km):
        dist = haversine(
            (row['Lat'], row['Lon']),
            central_point_coords,
            unit=Unit.KILOMETERS
        )

        return dist <= radius_km

    # Apply the function to each row in the DataFrame
    df['within_radius'] = df.apply(
        is_within_radius,
        axis=1,
        central_point_coords=central_point_coords,
        radius_km=radius_km
    )

    # Calculate the percentage of points within the radius
    within_radius_percentage = (df['within_radius'].sum() / len(df)) * 100

    return within_radius_percentage


def calc_time_diffs(df):
    # Calculate the time difference within each group
    time_diffs = df.groupby('VehicleNumber')['Time'].diff()

    # Convert the time differences to total seconds
    time_diffs_in_seconds = time_diffs.dt.total_seconds()

    # Assign the result to a new column in the dataframe
    df['TimeDiff'] = time_diffs_in_seconds


def calc_speeds_in_df(df):
    # Initialize a Speed column
    df['Speed'] = np.nan

    # Calculate the speed for each row
    for i in range(1, len(df)):
        # Check if it's the same bus
        if df['VehicleNumber'][i] == df['VehicleNumber'][i - 1]:
            # Calculate speed only if there is a valid time difference
            if df['TimeDiff'][i] > 0:
                df.at[i, 'Speed'] = calculate_speed(
                    df['Lat'][i - 1], df['Lon'][i - 1],
                    df['Lat'][i], df['Lon'][i],
                    df['TimeDiff'][i],
                    False
                )


def get_df(buses_loc_csv):
    # Load the CSV data into a DataFrame
    df = pd.read_csv(buses_loc_csv, delimiter=';')

    # Ensure data is sorted by VehicleNumber and Time
    df.sort_values(by=['VehicleNumber', 'Time'], inplace=True)
    df.reset_index(drop=True, inplace=True)

    # Convert the Time column to datetime
    df['Time'] = pd.to_datetime(df['Time'], errors='coerce')

    # Now, filter out those NaT values which indicate parsing errors
    invalid_times = df[df['Time'].isna()]

    # If you want to see which entries have invalid dates
    if df['Time'].isna().sum() > 0:
        print("There were some invalid dates, which will be omitted:")
        print(invalid_times)

    # If you want to handle these entries, you have a few options:
    df = df.dropna(subset=['Time'])

    df.reset_index(drop=True, inplace=True)

    calc_time_diffs(df)
    calc_speeds_in_df(df)

    return df


def call_all_analysis_and_visualization(
        df,
        speed_dist_png_path,
        speed_threshold,
        speeding_map_png_path,
        cols_cnt, rows_cnt,
        stops_coords_csv_path,
        lines_routes_json_path,
        stops_lines_json_path,
        schedules_json_path,
        centre_radius,
        distribution_png_path):
    places_with_many_fast_buses(
        df,
        speed_dist_png_path,
        speed_threshold,
        speeding_map_png_path,
        cols_cnt, rows_cnt
    )

    # on_time_analysis(
    #     df,
    #     stops_coords_csv_path,
    #     lines_routes_json_path,
    #     stops_lines_json_path,
    #     schedules_json_path
    # )

    print(
        'The percentage of buses being at most',
        centre_radius,
        'km from the central railway station =',
        get_percentage_within_radius(
            df,
            const.CENTRAL_STATION_COORDS,
            centre_radius
        )
    )
    visualizer.plot_buses_locs_around_point(
        df,
        const.CENTRAL_STATION_COORDS,
        const.CENTRAL_STATION_NAME,
        distribution_png_path
    )


# Returns the number of buses in the dataset faster than
# speed_threshold.
def analyze(
        speed_dist_png_path,
        speed_threshold,
        buses_loc_csv,
        lines_routes_json_path,
        schedules_json_path,
        stops_coords_csv_path,
        stops_lines_json_path,
        speeding_map_png_path,
        cols_cnt,
        rows_cnt,
        print_detailed_info,
        centre_radius,
        distribution_png_path):
    print('Analysis in progress...')

    df = get_df(buses_loc_csv)

    # Filter the data for rows where speed is greater than 50 km/h
    buses_above_50 = df[df['Speed'] > speed_threshold]

    if print_detailed_info:
        print("total buses cnt:", len(set(df['VehicleNumber'])))

        print("Speeding buses:")
        print(buses_above_50[['VehicleNumber', 'Speed', 'Time']])

    call_all_analysis_and_visualization(
        df,
        speed_dist_png_path,
        speed_threshold,
        speeding_map_png_path,
        cols_cnt,
        rows_cnt,
        stops_coords_csv_path,
        lines_routes_json_path,
        stops_lines_json_path,
        schedules_json_path,
        centre_radius,
        distribution_png_path
    )

    return len(set(buses_above_50['VehicleNumber']))
