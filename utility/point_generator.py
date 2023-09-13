import random
import json

def generate_random_points(n, X, Z):
    """
    Generates random 2D points within the specified dimensions.

    Args:
        n (int): The number of random points to generate.
        X (float): The maximum X-coordinate.
        Z (float): The maximum Z-coordinate.

    Returns:
        list: A list of dictionaries representing random points.
    """
    random_points = []
    for _ in range(n):
        point = {
            "Position": {
                "X": round(random.uniform(0, X), 2),
                "Y":round(random.uniform(0, Z), 2),
                "Z": 0.0
            }
        }
        random_points.append(point)
    return random_points

def write_json_file(data, filename):
    """
    Writes data to a JSON file.

    Args:
        data: The data to be written to the JSON file.
        filename: The name of the JSON file to be created.

    Returns:
        bool: True if the data was successfully written, False otherwise.
    """
    try:
        with open(filename, 'w') as json_file:
            json.dump(data, json_file, indent=4)
        return True
    except Exception as e:
        print(f"An error occurred: {str(e)}")
        return False

# Example usage:
n = 6400  # Number of random points
X = -40.0  # Maximum X-coordinate
Z = 100.0  # Maximum Z-coordinate

random_points = generate_random_points(n, X, Z)
random_points[0]["Position"]["X"] = 0.0
random_points[0]["Position"]["Y"] = 0.0
random_points[0]["Position"]["Z"] = 0.0

data_to_write = {"Instances": random_points}

file_name = "example.json"
if write_json_file(data_to_write, file_name):
    print(f"Data successfully written to {file_name}")
else:
    print(f"Failed to write data to {file_name}")
