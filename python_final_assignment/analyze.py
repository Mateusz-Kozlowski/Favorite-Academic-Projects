from analyzer import analyzer
import collecting_parser_base


def enhance_parser(parser):
    parser.add_argument(
        '--speed-threshold',
        type=int,
        required=True,
        help='Speeding threshold'
    )

    parser.add_argument(
        '--buses-loc-csv-path',
        required=True,
        help='Path to the csv file with buses locations data'
    )

    parser.add_argument(
        '--speed-dist-png-path',
        required=True,
        help='Path to the file speed distribution will be saved in'
    )

    parser.add_argument(
        '--speeding-map-png-path',
        required=True,
        help='Path to the file speeding map will be saved in'
    )

    parser.add_argument(
        '--cols-cnt',
        type=int,
        required=True,
        help='Columns count in the visualization of speeding'
    )

    parser.add_argument(
        '--rows-cnt',
        type=int,
        required=True,
        help='Rows count in the visualization of speeding'
    )

    parser.add_argument(
        '--detailed-info',
        type=int,
        required=True,
        help='Print detailed info (0 - no, 1 - yes)'
    )

    parser.add_argument(
        '--centre-radius',
        type=int,
        required=True,
        help='Radius in km the check how many buses are in center of Warsaw'
    )

    parser.add_argument(
        '--buses_dist-png-path',
        required=True,
        help='Distribution of buses around centre will be saved to the png'
    )

    return parser


def main():
    parser = enhance_parser(collecting_parser_base.setup_arg_parser())

    args = parser.parse_args()

    res = analyzer.analyze(
        args.speed_dist_png_path,
        args.speed_threshold,
        args.buses_loc_csv_path,
        args.lines_routes_json_path,
        args.schedules_json_path,
        args.stops_coords_csv_path,
        args.stops_lines_json_path,
        args.speeding_map_png_path,
        args.cols_cnt,
        args.rows_cnt,
        args.detailed_info,
        args.centre_radius,
        args.buses_dist_png_path
    )

    print(
        "The number of buses faster than",
        args.speed_threshold,
        "km/h:",
        res
    )

    print('Analyze done! Godbye World!')


if __name__ == '__main__':
    main()
