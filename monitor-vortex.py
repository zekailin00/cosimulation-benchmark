import time
import re
import sys

def monitor_log_file(log_file_path):
    """
    Monitors a log file for new lines containing specific messages and calculates the time difference between them.

    Args:
    log_file_path (str): Path to the log file to be monitored.
    """

    print("Looking at path: " + log_file_path)

    # Regular expression to match the lines containing the time message
    time_message_regex1 = r"UART0 is here"
    time_message_regex2 = r"no more active warps"

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
                    if re.search(time_message_regex1, line):
                        last_time_stamp = time.time()
                        print("Sim started: " + log_file_path)

                    elif re.search(time_message_regex2, line):
                        # Extract the current timestamp
                        current_time_stamp = time.time()

                        time_difference = current_time_stamp - last_time_stamp
                        print(f"Time difference: {time_difference} seconds: " + log_file_path)

    except FileNotFoundError:
        print(f"File not found: {log_file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage
# monitor_log_file("path/to/your/logfile.log")  # Replace with your log file path

# This script needs to be run in the background to continuously monitor the log file.

if __name__ == "__main__":
    monitor_log_file(sys.argv[1])