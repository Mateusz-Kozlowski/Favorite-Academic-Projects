import collecting_parser_base
from data_collector import schedules


def main():
    parser = collecting_parser_base.setup_arg_parser()

    args = parser.parse_args()

    schedules.get_schedules_data(
        args.stops_coords_csv_path,
        args.stops_lines_json_path,
        args.schedules_json_path,
        args.lines_routes_json_path
    )

    print("Collecting schedules done! Goodbye World!")


if __name__ == '__main__':
    main()
