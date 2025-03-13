import argparse


def setup_arg_parser():
    parser = argparse.ArgumentParser(
        description='Collect and analyze bus data.'
    )

    parser.add_argument(
        '--stops-coords-csv-path',
        required=True,
        help='Path to the stops coordinates CSV file'
    )
    parser.add_argument(
        '--stops-lines-json-path',
        required=True,
        help='Path to the stops lines JSON file'
    )
    parser.add_argument(
        '--schedules-json-path',
        required=True,
        help='Path to the schedules JSON file'
    )
    parser.add_argument(
        '--lines-routes-json-path',
        required=True,
        help='Path to the lines paths JSON file'
    )

    return parser
