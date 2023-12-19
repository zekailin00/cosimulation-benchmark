import time
import re

def monitor_log_file(log_file_path):
    """
    Monitors a log file for new lines containing specific messages and calculates the time difference between them.

    Args:
    log_file_path (str): Path to the log file to be monitored.
    """

    # Regular expression to match the lines containing the time message
    time_message_regex = r"RecordTime"

    print("begin")

    last_time_stamp = None

    try:
        with open(log_file_path, "r") as file:
            # Move to the end of the file
            # file.seek(0, 2)

            while True:
                # Read new line
                line = file.readline()

                # If line is not empty
                if line:
                    # Check if the line contains the time message
                    if re.search(time_message_regex, line):
                        # Extract the current timestamp
                        print("RecordTime")
                        current_time_stamp = time.time()

                        # If last timestamp is not None, calculate the difference
                        if last_time_stamp is not None:
                            time_difference = current_time_stamp - last_time_stamp
                            print(f"Time difference: {time_difference} seconds")

                        # Update the last timestamp
                        last_time_stamp = current_time_stamp
    except FileNotFoundError:
        print(f"File not found: {log_file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage
# monitor_log_file("path/to/your/logfile.log")  # Replace with your log file path

# This script needs to be run in the background to continuously monitor the log file.

if __name__ == "__main__":
    monitor_log_file("sample.log")