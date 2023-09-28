import random
import json

def generate_random_points(n, X, Z):
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
