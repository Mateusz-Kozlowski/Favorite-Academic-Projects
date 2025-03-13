import argparse
from datetime import datetime

from data_collector import buses_locations


def get_buses_loc_csv_path(args):
    current_hour = datetime.now().strftime('%H')
    time_str = f"{current_hour}_{args.duration}s"

    return f"analysis_data/buses_loc/{time_str}.csv"


def main():
    parser = argparse.ArgumentParser(
        description='Collect and analyze bus data.'
    )

    parser.add_argument(
        '--duration',
        type=int,
        required=True,
        help='Duration of gathering buses locations data'
    )

    args = parser.parse_args()

    buses_locations.get_buses_loc_data(
        args.duration,
        get_buses_loc_csv_path(args)
    )

    print("Buses location collected! Bye World!")


if __name__ == "__main__":
    main()
