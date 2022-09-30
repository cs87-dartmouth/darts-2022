import os
from sys import argv, exit, stderr, stdout
from datetime import datetime as dt
import subprocess
import pickle
import re
import math
import sys

def find_executable():
    '''
		Find the path to the DARTS executable.
		On windows, this is in the build/$<config> directory
		On mac/linux, the executable should be directly in the build directory
	'''

    root = os.getcwd()
    darts = ""
    if (os.name == 'nt'): #windows OS
        if os.path.exists(os.path.join(root, "build", "Release", "darts.exe")):
            darts = os.path.join(root, "build", "Release", "darts.exe")
        elif os.path.exists(os.path.join(root, "build", "darts.exe")):
            darts = os.path.join(root, "build", "darts.exe")
        else:
            print("Missing build dir\n")
            return None, None
    else: # mac/linux
        if os.path.exists(os.path.join(root, "build", "Release", "darts")):
            darts = os.path.join(root, "build", "Release", "darts")
        elif os.path.exists(os.path.join(root, "build", "darts")):
            darts = os.path.join(root, "build", "darts")
        else:
            print("Missing build dir\n")
            return None, None
    
    return darts, root

def render_image(exe_path, root_path):
    '''
        Render the image and retrieve the intersection metrics
    '''

    scene_dir = os.path.join(root_path, "scenes", "assignment2")
    scene_name = "leaderboard"
    scene_file = os.path.join(scene_dir, scene_name + ".json")

    start_time = dt.now()
    
    print("Starting render.... this may take a bit... ")
    proc = subprocess.Popen([exe_path, scene_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out = proc.communicate()
    print("Finished rendering image")

    stats_loc = out[0].decode('u8').find("Statistics:")

    intersections = out[0].decode('u8').find("Total intersection tests per ray")
    nodes = out[0].decode('u8').find("Nodes visited per ray")

    tri_str = out[0].decode('u8')[intersections::]
    tri_str_nodes = out[0].decode('u8')[nodes::]

    intersection_pattern = re.compile(r".*\s+\b(\d+)\b.*\s+\b(\d+)\b")

    match = intersection_pattern.match(tri_str.strip())
    match_nodes = intersection_pattern.match(tri_str_nodes.strip())

    files = [os.path.join(scene_dir, fname) for fname in os.listdir(scene_dir) if fname.startswith(scene_name) and fname.endswith(".png")]

    files.sort(key=lambda x: os.path.getmtime(x))

    most_recent_img = files[-1]
    end_time = dt.now()

    intersections = float(match.group(1)) / float(match.group(2))
    nodes = float(match_nodes.group(1)) / float(match_nodes.group(2))

    return most_recent_img, intersections, nodes, start_time, end_time

def create_hash(img_path, intersections, nodes, publicize_results, time_start, time_end):
    outfile = open('leaderboard.dat', 'wb')
    img = open(img_path, 'rb')

    pickle.dump([intersections, nodes, publicize_results, time_start, time_end, img.read()], outfile, pickle.HIGHEST_PROTOCOL)
    outfile.close()

def main():
    exe_path, scene_path = find_executable()
    if not exe_path:
        print("Error: cannot find DARTS executable path, please ensure that you are running the leaderboard.exe script from the darts directory, and that the darts executable has been built.", file=stderr)
        exit(1)

    print("Executable located at:", exe_path)
    print("Scenes located at:", scene_path)
    print()

    # Note: make this True if you want your results to be displayed on the class website leaderboard
    publicize_results = False
    if len(sys.argv) > 1:
        publicize_cmd = sys.argv[1]
        if publicize_cmd.strip() == "public":
            publicize_results = True

    img_path, intersections, nodes, time_start, time_end = render_image(exe_path, scene_path)

    print("Image path:", img_path)
    print()
    print("Total intersection tests per ray:", math.floor(intersections*100.0)/100.0)
    print("Nodes visited per ray:", math.floor(nodes*100.0)/100.0)
    print("Figure of merit:", math.floor(nodes*intersections*100.0)/100.0)
    print()

    create_hash(img_path, intersections, nodes, publicize_results, time_start, time_end)

if __name__ == "__main__":
	main()
