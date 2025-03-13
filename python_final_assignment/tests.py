from analyzer import analyzer


def main():
    tests = {
        "analysis_data/buses_loc/bialoleka.csv": 1,
        "analysis_data/buses_loc/my_big_test.csv": 2,
        "analysis_data/buses_loc/precise_test.csv": 1,
        "analysis_data/buses_loc/ursus_wilanow.csv": 3
    }

    for test_key in tests:
        print(test_key)

        test_res = analyzer.analyze(
            "tmp/tmp",
            50,
            test_key,
            "analysis_data/lines_routes.json",
            "analysis_data/schedules.json",
            "analysis_data/stops_coords.csv",
            "analysis_data/stops_lines_json_path.csv",
            "tmp/tmp",
            2,
            2,
            True,
            5,
            "tmp/tmp"
        )

        assert test_res == tests[test_key]

        print('--------------------------------------------------------------')

    print('Test done! Bye Wolrd!')


if __name__ == '__main__':
    main()
