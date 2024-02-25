import os
from itertools import groupby

dir = "output/"

file_names = []

for entry in os.scandir(dir):
    if (entry.name.split(".")[-1] == "txt"):
        file_names.append(entry.name)

def get_input_name(s):
    parts = s.split("_")
    return "_".join(parts[:len(parts) - 5])

def get_method(s):
    parts = s.split("_")
    return "_".join(parts[-4:-2])

def get_vertex_count(s):
    return int(s.split("_")[-2])

def metadata(f):
    lines = map(lambda line: line.split(" ")[-1], f.read().splitlines())
    return list(lines)


file_names.sort()
for input, group1 in groupby(file_names, get_input_name):
    print("\\begin{axis}[")
    print(f"title=\u007b{input}\u007d")
    print("]")
    for method, group2 in groupby(sorted(group1, key=get_method), get_method):
        name, angle = method.split("_")
        if angle != "0.523599":
            continue
        print("\\addplot coordinates {")
        for member in sorted(group2, key=get_vertex_count, reverse=True):
            with open(dir + member) as f:
                print(f"({get_vertex_count(member)}, {metadata(f)[1]})", end=" ")
        print()
        print("};")
        print(f"\\addlegendentry\u007b{name}\u007d")
            # print(member)
    print("\\end{axis}")

# for name in file_names:
#     print(name)
#     print(get_input_name(name))


