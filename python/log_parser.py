"""Parse logs and graph"""

import numpy as np
import pandas as pd


def parse_log(file_name: str):
    """Parse a log file and convert to a timeseries data structure.

    Args:
        file_name (str): file name of the log file

    Returns:
        timeseries
    """
    data = pd.DataFrame(
        {
            'tank_temp': [],
            'heat_downstairs': [],
            'heat_upstairs': [],
            'hotwater_valve_open': [],
            'rads_valve_open': [],
            'boiler_req': [],
            'pump_req': [],
            'hotwater_enable': [],
        },
    )
    
    with open(file_name, 'r') as log_file:
        for line in log_file:
            if line == 'Boiler on':
                continue

            values = line.strip().split(' ')

            data.append(
                [
                    values[0],
                    values[1],
                    values[2],
                    values[3],
                    values[5],
                    values[6],
                    values[7],
                ],
            )

    print(data)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('log_file', type='str', help='foo help')
    args = parser.parse_args()

    parse_log(args.log_file)
